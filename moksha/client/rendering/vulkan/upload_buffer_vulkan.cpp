#ifdef ME_VULKAN

#include "upload_buffer_vulkan.h"

#include <cstdint>
#include <cstring>

#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"
#include "utils_vulkan.h"

bool ME::UploadBufferVulkan::Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, uint32_t inElementCount,
                                  uint32_t inElementSize, Kind inKind) {
    device = inDevice;
    kind = inKind;
    elementCount = inElementCount;
    elementSize = inElementSize;

    VkPhysicalDeviceProperties props = {};
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    const bool storage = kind == Kind::Storage;
    const uint32_t align = static_cast<uint32_t>(storage ? props.limits.minStorageBufferOffsetAlignment
                                                         : props.limits.minUniformBufferOffsetAlignment);
    const uint32_t maxRange = storage ? props.limits.maxStorageBufferRange : props.limits.maxUniformBufferRange;
    const VkBufferUsageFlags usage = storage ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // A uniform element starts on the alignment, as each is a descriptor's target. Storage elements stay packed at
    // the shader's array stride; only each frame's copy starts on the alignment.
    stride = storage ? elementSize : (elementSize + align - 1) / align * align;
    const uint32_t usedFrameSize = stride * elementCount;
    frameSize = (usedFrameSize + align - 1) / align * align;
    const VkDeviceSize size = static_cast<VkDeviceSize>(frameSize) * Constants::FramesInFlight;

    const uint32_t range = GetDescriptorRange();
    if (range > maxRange) {
        ME::LogError("UploadBufferVulkan: a descriptor range of ", range, " bytes exceeds the device's limit of ",
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

void ME::UploadBufferVulkan::CopyElements(uint32_t frameIndex, const void* data, uint32_t count) {
    if (mapped == nullptr || data == nullptr || frameIndex >= Constants::FramesInFlight || count == 0) {
        return;
    }
    const uint32_t copyCount = count < elementCount ? count : elementCount;
    const uint8_t* source = static_cast<const uint8_t*>(data);
    if (stride == elementSize) {
        std::memcpy(mapped + GetOffset(frameIndex, 0), source, static_cast<size_t>(elementSize) * copyCount);
        return;
    }
    // Padded uniform elements: one at a time.
    for (uint32_t i = 0; i < copyCount; ++i) {
        std::memcpy(mapped + GetOffset(frameIndex, i), source + static_cast<size_t>(i) * elementSize, elementSize);
    }
}

VkBuffer ME::UploadBufferVulkan::GetBuffer() const {
    return buffer;
}

uint32_t ME::UploadBufferVulkan::GetDescriptorRange() const {
    return kind == Kind::Storage ? elementSize * elementCount : elementSize;
}

uint32_t ME::UploadBufferVulkan::GetOffset(uint32_t frameIndex, uint32_t elementIndex) const {
    return frameIndex * frameSize + elementIndex * stride;
}

#endif  // ME_VULKAN
