#ifdef ME_VULKAN

#include "texture_vulkan.h"

#include "moksha/logging/logging.h"
#include "utils_vulkan.h"

bool ME::TextureVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue,
                             VkCommandPool uploadPool, const char* path) {
    device = inDevice;
    Load(path);
    if (data == nullptr || width == 0 || height == 0) {
        ME::LogError("TextureVulkan: can't load ", path);
        return false;
    }

    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_SRGB;
    info.extent = {width, height, 1};
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkResult result = vkCreateImage(device, &info, nullptr, &image);
    if (result != VK_SUCCESS) {
        ME::LogError("TextureVulkan: failed to create image for ", path);
        return false;
    }

    VkMemoryRequirements req = {};
    vkGetImageMemoryRequirements(device, image, &req);

    const uint32_t typeIndex =
        ME::VulkanUtils::FindMemoryType(physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (typeIndex == UINT32_MAX) {
        ME::LogError("TextureVulkan: failed to find a suitable memory type for ", path);
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = req.size;
    allocInfo.memoryTypeIndex = typeIndex;
    result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
    if (result != VK_SUCCESS) {
        ME::LogError("TextureVulkan: failed to allocate memory for ", path);
        return false;
    }
    result = vkBindImageMemory(device, image, memory, 0);
    if (result != VK_SUCCESS) {
        ME::LogError("TextureVulkan: failed to bind image memory for ", path);
        return false;
    }

    if (!Upload(physicalDevice, queue, uploadPool)) {
        ME::LogError("TextureVulkan: upload failed for ", path);
        return false;
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        ME::LogError("TextureVulkan: vkCreateImageView failed (", static_cast<int>(result), ") for ", path);
        return false;
    }

    return true;
}

void ME::TextureVulkan::End() {
    // The view was made from the image, so it goes first.
    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
    if (image != VK_NULL_HANDLE) {
        vkDestroyImage(device, image, nullptr);
        image = VK_NULL_HANDLE;
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

VkImage ME::TextureVulkan::GetImage() const {
    return image;
}

VkImageView ME::TextureVulkan::GetImageView() const {
    return imageView;
}

bool ME::TextureVulkan::Upload(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool) {
    // The image is R8G8B8A8; the cook writes 4-byte RGBA pixels.
    if (channels != 4) {
        ME::LogError("TextureVulkan: expected RGBA8 pixels, got ", static_cast<int>(channels), " channels");
        return false;
    }
    const VkDeviceSize size = static_cast<VkDeviceSize>(width) * height * 4;

    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    if (!ME::VulkanUtils::CreateBuffer(physicalDevice, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, data, &staging,
                                       &stagingMemory)) {
        return false;
    }

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (!ME::VulkanUtils::BeginOneTimeCommands(device, uploadPool, &cmd)) {
        ME::VulkanUtils::DestroyBuffer(device, &staging, &stagingMemory);
        return false;
    }

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // Barrier 1: undefined → ready to be copied into.
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(cmd, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Barrier 2: copied into → ready to be sampled by the fragment shader.
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    const bool ok = ME::VulkanUtils::EndOneTimeCommands(device, queue, uploadPool, cmd);
    ME::VulkanUtils::DestroyBuffer(device, &staging,
                                   &stagingMemory);  // the GPU is done with it: EndOneTimeCommands waited
    return ok;
}

#endif  // ME_VULKAN
