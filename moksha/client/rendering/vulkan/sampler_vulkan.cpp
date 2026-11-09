#ifdef ME_VULKAN

#include "sampler_vulkan.h"

#include "moksha/logging/logging.h"

VkSampler ME::SamplerVulkan::CreatePointClamp(VkDevice device) {
    return Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

VkSampler ME::SamplerVulkan::Create(VkDevice device, VkFilter filter, VkSamplerAddressMode address) {
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = filter;
    info.minFilter = filter;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    info.addressModeU = address;
    info.addressModeV = address;
    info.addressModeW = address;
    info.mipLodBias = 0.0f;
    info.anisotropyEnable = VK_FALSE;
    info.maxAnisotropy = 1.0f;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.minLod = 0.0f;
    info.maxLod = 0.0f;
    // Only read by CLAMP_TO_BORDER.
    info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler = VK_NULL_HANDLE;
    const VkResult result = vkCreateSampler(device, &info, nullptr, &sampler);
    if (result != VK_SUCCESS) {
        ME::LogError("SamplerVulkan: vkCreateSampler failed (", static_cast<int>(result), ")");
        return VK_NULL_HANDLE;
    }
    return sampler;
}

#endif  // ME_VULKAN
