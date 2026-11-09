#pragma once
#ifdef ME_VULKAN

/**
 * Vulkan's upload buffer, the counterpart of UploadBufferDX: constant data the CPU writes every frame and the GPU
 * reads directly. Holds one copy per frame in flight, each element on the device's uniform offset alignment, so an
 * element is selected with a dynamic offset. Stays mapped for its whole life.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ME {

class UploadBufferVulkan {
   public:
    UploadBufferVulkan() = default;
    ~UploadBufferVulkan() = default;

    UploadBufferVulkan(const UploadBufferVulkan&) = delete;
    UploadBufferVulkan& operator=(const UploadBufferVulkan&) = delete;

    // Room for elementCount elements of elementSize bytes, per frame in flight. False (logged) on failure; call
    // End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, uint32_t inElementCount, uint32_t inElementSize);
    // The GPU must be done with the buffer (call after vkDeviceWaitIdle).
    void End();

    // Copies one element (elementSize bytes) into a frame's copy.
    void CopyElement(uint32_t frameIndex, uint32_t elementIndex, const void* data);

    VkBuffer GetBuffer() const;

    // Size of one element, for the descriptor's range.
    uint32_t GetElementSize() const;

    // Dynamic offset of an element in a frame's copy.
    uint32_t GetOffset(uint32_t frameIndex, uint32_t elementIndex) const;

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    uint8_t* mapped = nullptr;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;  // bytes of real data in one element
    uint32_t stride = 0;       // elementSize rounded up to the alignment
};

}  // namespace ME

#endif  // ME_VULKAN
