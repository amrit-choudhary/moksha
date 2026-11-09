#pragma once
#ifdef ME_VULKAN

/**
 * Pipeline layouts for Vulkan, the counterpart of RootSigDx: what constants and textures each kind of pipeline's
 * shaders read. The caller owns each layout and destroys it with vkDestroyPipelineLayout.
 */

#include <vulkan/vulkan.h>

namespace ME {

class PipelineLayoutVulkan {
   public:
    PipelineLayoutVulkan() = delete;

    // Set 0: the per-pass constants (one dynamic uniform buffer, vertex stage). Null on failure (logged).
    static VkDescriptorSetLayout CreateSetLayoutPerPass(VkDevice device);
    // Set 1: the per-sprite constants (one dynamic uniform buffer, vertex stage). Null on failure (logged).
    static VkDescriptorSetLayout CreateSetLayoutPerSprite(VkDevice device);

    // Layout for 2D rendering without instancing, with texture atlases: set 0 per pass, set 1 per sprite.
    static VkPipelineLayout CreateLayout2DAtlas(VkDevice device, VkDescriptorSetLayout perPass,
                                                VkDescriptorSetLayout perSprite);
};

}  // namespace ME

#endif  // ME_VULKAN
