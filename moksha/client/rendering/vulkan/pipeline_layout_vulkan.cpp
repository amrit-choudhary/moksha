#ifdef ME_VULKAN

#include "pipeline_layout_vulkan.h"

#include "moksha/logging/logging.h"

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateSetLayoutPerPass(VkDevice device) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &binding;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &setLayout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreateDescriptorSetLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateSetLayoutPerSprite(VkDevice device) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &binding;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &setLayout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreateDescriptorSetLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateSetLayoutTexture(VkDevice device, VkSampler sampler) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = &sampler;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &binding;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &setLayout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreateDescriptorSetLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

VkPipelineLayout ME::PipelineLayoutVulkan::CreateLayout2DAtlas(VkDevice device, VkDescriptorSetLayout perPass,
                                                               VkDescriptorSetLayout perSprite,
                                                               VkDescriptorSetLayout texture) {
    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 3;
    VkDescriptorSetLayout setLayouts[3] = {perPass, perSprite, texture};
    info.pSetLayouts = setLayouts;

    VkPipelineLayout layout = VK_NULL_HANDLE;
    const VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &layout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreatePipelineLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return layout;
}

#endif  // ME_VULKAN
