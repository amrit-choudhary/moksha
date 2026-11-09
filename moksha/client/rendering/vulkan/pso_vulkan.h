#pragma once
#ifdef ME_VULKAN

/**
 * Pipeline state for Vulkan, the counterpart of PSODirectX: graphics pipelines built for subpass 0 of a render pass
 * from shaders/glsl/<name>.vert/.frag. The caller owns each pipeline and destroys it with vkDestroyPipeline.
 */

#include <vulkan/vulkan.h>

namespace ME {

class PSOVulkan {
   public:
    PSOVulkan() = delete;

    // Pipeline for 2D rendering without instancing, with texture atlases. layout comes from PipelineLayoutVulkan; keep
    // it while the pipeline is used. Null on failure (logged).
    static VkPipeline CreatePSO2DAtlas(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
                                       const char* shaderName);
};

}  // namespace ME

#endif  // ME_VULKAN
