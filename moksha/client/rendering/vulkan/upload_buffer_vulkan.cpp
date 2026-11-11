#ifdef ME_VULKAN

#include "upload_buffer_vulkan.h"

#include <cstdint>
#include <cstring>

#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"
#include "utils_vulkan.h"

bool ME::UploadBufferVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, uint32_t inElementCount,
                                  uint32_t inElementSize, Kind kind) {
    device = inDevice;
    elementCount = inElementCount;
    elementSize = inElementSize;

    VkPhysicalDeviceProperties props = {};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    const bool storage = kind == Kind::Storage;
    const uint32_t align = static_cast<uint32_t>(storage ? props.limits.minStorageBufferOffsetAlignment
                                                         : props.limits.minUniformBufferOffsetAlignment);
    const uint32_t maxRange = storage ? props.limits.maxStorageBufferRange : props.limits.maxUniformBufferRange;
    const VkBufferUsageFlags usage = storage ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    stride = (elementSize + align - 1) / align * align;
    const VkDeviceSize size = static_cast<VkDeviceSize>(stride) * elementCount * Constants::FramesInFlight;

    if (elementSize > maxRange) {
        ME::LogError("UploadBufferVulkan: element of ", elementSize, " bytes exceeds the device's range limit of ",
                     maxRange);
        return false;
    }

    if (!ME::VulkanUtils::CreateBuffer(physicalDevice, device, size, usage, nullptr, &buffer, &memory)) {
        return false;
    }

    void* ptr = nullptr;
    const VkResult result = vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &ptr);
    if (result != VK_SUCCESS || ptr == nullptr) {
        ME::LogError("UploadBufferVulkan: vkMapMemory failed (", static_cast<int>(result), ")");
        return false;
    }
    mapped = static_cast<uint8_t*>(ptr);
    return true;
}

void ME::UploadBufferVulkan::End() {
    if (mapped != nullptr) {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }
    ME::VulkanUtils::DestroyBuffer(device, &buffer, &memory);
}

void ME::UploadBufferVulkan::CopyElement(uint32_t frameIndex, uint32_t elementIndex, const void* data) {
    if (mapped == nullptr || data == nullptr || frameIndex >= Constants::FramesInFlight ||
        elementIndex >= elementCount) {
        return;
    }
    std::memcpy(mapped + GetOffset(frameIndex, elementIndex), data, elementSize);
}

VkBuffer ME::UploadBufferVulkan::GetBuffer() const {
    return buffer;
}

uint32_t ME::UploadBufferVulkan::GetElementSize() const {
    return elementSize;
}

uint32_t ME::UploadBufferVulkan::GetOffset(uint32_t frameIndex, uint32_t elementIndex) const {
    return (frameIndex * elementCount + elementIndex) * stride;
}

#endif  // ME_VULKAN
