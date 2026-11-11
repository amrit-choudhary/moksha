#ifdef ME_VULKAN

#include "pipeline_layout_vulkan.h"

#include "moksha/logging/logging.h"

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateDescriptorSetLayoutPerPass(VkDevice device) {
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;  // CBPerPass
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

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateDescriptorSetLayoutPerSprite(VkDevice device) {
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;  // CBPerSprite
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    bindings[1] = bindings[0];
    bindings[1].binding = 1;  // TextureAtlasProperties

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 2;
    info.pBindings = bindings;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &setLayout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreateDescriptorSetLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateDescriptorSetLayoutInstanced(VkDevice device) {
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;  // SpriteInstanceData[]
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    bindings[1].binding = 1;  // TextureAtlasProperties
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 2;
    info.pBindings = bindings;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    const VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &setLayout);
    if (result != VK_SUCCESS) {
        ME::LogError("PipelineLayoutVulkan: vkCreateDescriptorSetLayout failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

VkDescriptorSetLayout ME::PipelineLayoutVulkan::CreateDescriptorSetLayoutTexture(VkDevice device, VkSampler sampler) {
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

VkPipelineLayout ME::PipelineLayoutVulkan::CreateLayout2DInstancedAtlas(VkDevice device, VkDescriptorSetLayout perPass,
                                                                        VkDescriptorSetLayout perDraw,
                                                                        VkDescriptorSetLayout texture) {
    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 3;
    VkDescriptorSetLayout setLayouts[3] = {perPass, perDraw, texture};
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
