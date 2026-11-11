#pragma once
#ifdef ME_VULKAN

/**
 * Takes a cross platform UI scene and creates its Vulkan resources: the counterpart of SceneUIDX. Owns the UI textures
 * (atlas at slot 0, font at slot 1), the UI sprite and text pass constants, the atlas properties, and the UI sprite and
 * text instance buffers, sized at their maximum counts so UI elements can be added after Init; each frame copies and
 * draws only the live counts. The set layouts are the renderer's and must outlive the scene.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

#include "moksha/client/rendering/vulkan/texture_vulkan.h"
#include "moksha/client/rendering/vulkan/upload_buffer_vulkan.h"
#include "moksha/shared/misc/engine_limits.h"
#include "scene_ui.h"

namespace ME {

class SceneUIVulkan {
   public:
    SceneUIVulkan() = default;
    ~SceneUIVulkan() = default;

    SceneUIVulkan(const SceneUIVulkan&) = delete;
    SceneUIVulkan& operator=(const SceneUIVulkan&) = delete;

    // Creates the UI scene's textures, buffers and sets; uiScene must outlive this object. False (logged) on failure;
    // call End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue, VkCommandPool uploadPool,
              VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout instancedLayout,
              VkDescriptorSetLayout textureLayout, ME::SceneUI* uiScene);

    // The GPU must be done with the scene (call after vkDeviceWaitIdle).
    void End();

    // Re-reads the UI scene's live counts (elements can be added after Init), capped at the buffers' capacity. Call
    // after SceneUI::Update.
    void Update();

    // Writes the pass constants (screenDimension: packed window size), the atlas properties and the live instances
    // into frameIndex's copies.
    void UploadFrame(uint32_t frameIndex, uint32_t screenDimension);

    // Dynamic offsets for this frame's copies.
    uint32_t GetUISpritePassOffset(uint32_t frameIndex) const;
    uint32_t GetTextPassOffset(uint32_t frameIndex) const;
    uint32_t GetAtlasOffset(uint32_t frameIndex, uint32_t atlasIndex) const;
    uint32_t GetUISpriteInstanceOffset(uint32_t frameIndex) const;
    uint32_t GetTextInstanceOffset(uint32_t frameIndex) const;

    uint32_t spriteTextureCount = 0;
    uint32_t atlasCount = 0;
    uint32_t uiSpriteInstanceCount = 0;  // live UI sprite instances, capped at MaxUISpriteInstanceDataCount
    uint32_t textInstanceCount = 0;      // live glyph instances, capped at MaxTextInstanceDataCount

    VkDescriptorSet uiSpritePassSet = VK_NULL_HANDLE;      // set 0 for the UI sprite pass
    VkDescriptorSet textPassSet = VK_NULL_HANDLE;          // set 0 for the UI text pass
    VkDescriptorSet uiSpriteInstanceSet = VK_NULL_HANDLE;  // set 1: UI sprite instances and atlas properties
    VkDescriptorSet textInstanceSet = VK_NULL_HANDLE;      // set 1: glyph instances and atlas properties
    // Set 2 per texture; null where the texture failed to load.
    VkDescriptorSet textureSets[Constants::MaxSpriteTextureCount] = {};

   private:
    VkDevice device = VK_NULL_HANDLE;
    ME::SceneUI* scene = nullptr;

    ME::TextureVulkan spriteTextures[Constants::MaxSpriteTextureCount];
    UploadBufferVulkan uiSpritePassBuffer;
    UploadBufferVulkan textPassBuffer;
    UploadBufferVulkan atlasBuffer;
    UploadBufferVulkan uiSpriteInstanceBuffer;
    UploadBufferVulkan textInstanceBuffer;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    // Loads every UI texture. A texture that fails to load is logged and its pass skipped; it doesn't fail the scene.
    bool MakeSpriteTextures(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool);

    // Creates the pass, atlas and instance buffers; the instance buffers at their maximum counts. False (logged) on
    // failure.
    bool MakeBuffers(VkPhysicalDevice physicalDevice);

    // Creates the pool and the sets: two set 0s, two set 1s, and a set 2 per texture that loaded. False (logged) on
    // failure.
    bool MakeDescriptorSets(VkDescriptorSetLayout perPassLayout, VkDescriptorSetLayout instancedLayout,
                            VkDescriptorSetLayout textureLayout);
};

}  // namespace ME

#endif  // ME_VULKAN
