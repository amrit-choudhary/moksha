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

    // Layout for 2D rendering without instancing, with texture atlases. Null on failure (logged).
    static VkPipelineLayout CreateLayout2DAtlas(VkDevice device);
};

}  // namespace ME

#endif  // ME_VULKAN
