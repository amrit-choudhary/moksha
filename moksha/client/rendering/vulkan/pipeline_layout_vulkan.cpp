#ifdef ME_VULKAN

#include "pipeline_layout_vulkan.h"

#include "moksha/logging/logging.h"

VkPipelineLayout ME::PipelineLayoutVulkan::CreateLayout2DAtlas(VkDevice device) {
    // TODO: no descriptor set layouts yet; the shaders read no constants or textures.
    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout layout = VK_NULL_HANDLE;
    const VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &layout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreatePipelineLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return layout;
}

#endif  // ME_VULKAN
