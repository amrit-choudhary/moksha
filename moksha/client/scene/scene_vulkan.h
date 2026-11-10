#pragma once
#ifdef ME_VULKAN

/**
 * Takes a cross platform scene and creates its Vulkan resources: the counterpart of SceneDX. Owns the sprite textures,
 * one buffer for all per-sprite constants, one for all atlas properties, and the descriptor sets pointing at them,
 * written once in Init; each draw picks its sprite and atlas with dynamic offsets. The set layouts are the renderer's
 * and must outlive the scene.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

#include "moksha/client/rendering/vulkan/texture_vulkan.h"
#include "moksha/client/rendering/vulkan/upload_buffer_vulkan.h"
#include "moksha/shared/misc/engine_limits.h"
#include "scene.h"

namespace ME {

class SceneVulkan {
   public:
    SceneVulkan() = default;
    ~SceneVulkan() = default;

    SceneVulkan(const SceneVulkan&) = delete;
    SceneVulkan& operator=(const SceneVulkan&) = delete;

    // Creates the scene's textures, buffers and sets; gameScene must outlive this object. False (logged) on failure;
    // call End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue, VkCommandPool uploadPool,
              VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout perSpriteLayout,
              VkDescriptorSetLayout textureLayout, ME::Scene* gameScene);

    // The GPU must be done with the scene (call after vkDeviceWaitIdle).
    void End();

    // Writes the camera, the atlas properties and every sprite's constants into frameIndex's copies.
    void UploadFrame(uint32_t frameIndex);

    // Dynamic offsets for this frame's copies.
    uint32_t GetPerPassOffset(uint32_t frameIndex) const;
    uint32_t GetSpriteOffset(uint32_t frameIndex, uint32_t spriteIndex) const;
    uint32_t GetAtlasOffset(uint32_t frameIndex, uint32_t atlasIndex) const;

    ME::Span<ME::SpriteRenderer> spriteRenderers;
    uint32_t spriteCount = 0;  // sprites drawn: spriteRenderers.count, capped at MaxSpriteRendererCount
    uint32_t spriteTextureCount = 0;
    uint32_t atlasCount = 0;

    VkDescriptorSet perPassSet = VK_NULL_HANDLE;    // set 0
    VkDescriptorSet perSpriteSet = VK_NULL_HANDLE;  // set 1: sprite constants and atlas properties
    // Set 2 per sprite texture; null where the texture failed to load.
    VkDescriptorSet textureSets[Constants::MaxSpriteTextureCount] = {};

   private:
    VkDevice device = VK_NULL_HANDLE;
    ME::Scene* scene = nullptr;

    ME::TextureVulkan spriteTextures[Constants::MaxSpriteTextureCount];
    UploadBufferVulkan perPassBuffer;
    UploadBufferVulkan perSpriteBuffer;
    UploadBufferVulkan atlasBuffer;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    // Loads every sprite texture. A texture that fails to load is logged and its sprites skipped; it doesn't fail the
    // scene.
    bool MakeSpriteTextures(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool);

    // Creates the per-pass, per-sprite and atlas buffers, sized from the scene. False (logged) on failure.
    bool MakeBuffers(VkPhysicalDevice physicalDevice);

    // Creates the pool and the sets: set 0, set 1, and a set 2 per texture that loaded. False (logged) on failure.
    bool MakeDescriptorSets(VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout perSpriteLayout,
                            VkDescriptorSetLayout textureLayout);
};

}  // namespace ME

#endif  // ME_VULKAN
