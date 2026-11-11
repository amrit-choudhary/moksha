#pragma once
#ifdef ME_VULKAN

/**
 * Takes a cross platform scene and creates its Vulkan resources: the counterpart of SceneDX. Owns the sprite textures,
 * one buffer for all per-sprite constants, one for all atlas properties, a storage buffer per instanced set, and the
 * descriptor sets pointing at them, written once in Init; each draw picks its frame copy, sprite and atlas with dynamic
 * offsets. The set layouts are the renderer's and must outlive the scene.
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
              VkDescriptorSetLayout instancedLayout, VkDescriptorSetLayout textureLayout, ME::Scene* gameScene);

    // The GPU must be done with the scene (call after vkDeviceWaitIdle).
    void End();

    // Writes the camera, the atlas properties, every sprite's constants and both instance arrays into frameIndex's
    // copies.
    void UploadFrame(uint32_t frameIndex);

    // Dynamic offsets for this frame's copies.
    uint32_t GetPerPassOffset(uint32_t frameIndex) const;
    uint32_t GetSpriteOffset(uint32_t frameIndex, uint32_t spriteIndex) const;
    uint32_t GetAtlasOffset(uint32_t frameIndex, uint32_t atlasIndex) const;
    uint32_t GetInstanceOffset0(uint32_t frameIndex) const;  // the instance array's frame copy
    uint32_t GetInstanceOffset1(uint32_t frameIndex) const;

    ME::Span<ME::SpriteRenderer> spriteRenderers;
    uint32_t spriteCount = 0;  // sprites drawn: spriteRenderers.count, capped at MaxSpriteRendererCount
    uint32_t spriteTextureCount = 0;
    uint32_t atlasCount = 0;

    VkDescriptorSet perPassSet = VK_NULL_HANDLE;    // set 0
    VkDescriptorSet perSpriteSet = VK_NULL_HANDLE;  // set 1: sprite constants and atlas properties
    // Set 2 per sprite texture; null where the texture failed to load.
    VkDescriptorSet textureSets[Constants::MaxSpriteTextureCount] = {};

    // Instanced sets 0 and 1: the Scene's arrays, the instances drawn (0 = the set is skipped), and the set 1 holding
    // the instance array and the atlas properties.
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers0;
    ME::SpriteRendererInstanceData* spriteInstanceData0 = nullptr;
    uint32_t instanceCount0 = 0;
    VkDescriptorSet spriteInstanceSet0 = VK_NULL_HANDLE;

    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers1;
    ME::SpriteRendererInstanceData* spriteInstanceData1 = nullptr;
    uint32_t instanceCount1 = 0;
    VkDescriptorSet spriteInstanceSet1 = VK_NULL_HANDLE;

   private:
    VkDevice device = VK_NULL_HANDLE;
    ME::Scene* scene = nullptr;

    ME::TextureVulkan spriteTextures[Constants::MaxSpriteTextureCount];
    UploadBufferVulkan perPassBuffer;
    UploadBufferVulkan perSpriteBuffer;
    UploadBufferVulkan atlasBuffer;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    UploadBufferVulkan spriteInstanceBuffer0;
    UploadBufferVulkan spriteInstanceBuffer1;

    // Loads every sprite texture. A texture that fails to load is logged and its sprites skipped; it doesn't fail the
    // scene.
    bool MakeSpriteTextures(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool);

    // Creates the per-pass, per-sprite and atlas buffers, sized from the scene. False (logged) on failure.
    bool MakeBuffers(VkPhysicalDevice physicalDevice);

    // Creates the pool and the sets: set 0, set 1, a set 1 per non-empty instanced set, and a set 2 per texture that
    // loaded. False (logged) on failure.
    bool MakeDescriptorSets(VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout perSpriteLayout,
                            VkDescriptorSetLayout instancedLayout, VkDescriptorSetLayout textureLayout);

    // Creates a storage buffer per non-empty instanced set, each set on its own. False (logged) on failure.
    bool MakeSpriteInstanceBuffer(VkPhysicalDevice physicalDevice);

    // Allocates one instanced set 1 from allocInfo (its layout already set) and points it at instanceBuffer and the
    // atlas buffer. name is for the log. False (logged) on failure.
    bool WriteInstanceSet(const VkDescriptorSetAllocateInfo& allocInfo, const UploadBufferVulkan& instanceBuffer,
                          const VkDescriptorBufferInfo& atlasInfo, VkDescriptorSet* outSet, const char* name);
};

}  // namespace ME

#endif  // ME_VULKAN
