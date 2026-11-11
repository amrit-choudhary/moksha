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

    // Pipeline for 2D rendering without instancing, with texture atlases, straight alpha blending and depth. layout
    // comes from PipelineLayoutVulkan; keep it while the pipeline is used. Null on failure (logged).
    static VkPipeline CreatePSO2DAtlas(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
                                       const char* shaderName);

    // Pipeline for 2D rendering with instancing, with texture atlases, straight alpha blending and depth. layout
    // comes from PipelineLayoutVulkan; keep it while the pipeline is used. Null on failure (logged).
    static VkPipeline CreatePSO2DInstancedAtlas(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
                                                const char* shaderName);

    // Pipelines for the UI sprite and UI text passes: instanced, texture atlases, straight alpha blending, no depth
    // test or writes, so UI draws over the scene in order. layout as above. Null on failure (logged).
    static VkPipeline CreatePSO2DUISprite(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
                                          const char* shaderName);
    static VkPipeline CreatePSO2DUIText(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
                                        const char* shaderName);
};

}  // namespace ME

#endif  // ME_VULKAN
