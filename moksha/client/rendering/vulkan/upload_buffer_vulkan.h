#pragma once
#ifdef ME_VULKAN

/**
 * Vulkan's upload buffer, the counterpart of UploadBufferDX: data the CPU writes every frame and the GPU reads
 * directly, as a uniform or storage buffer. Holds one copy per frame in flight, each starting on the device's offset
 * alignment for that kind, so a frame's copy is selected with a dynamic offset. Uniform elements are padded to the
 * alignment (each is its own descriptor target); storage elements are packed (an array the shader indexes). Stays
 * mapped for its whole life.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ME {

class UploadBufferVulkan {
   public:
    // What the buffer backs: picks the usage flag, the offset alignment, the element packing and the range limit.
    enum class Kind : uint8_t {
        Uniform,  // uniform buffer: small constants (per pass, per sprite, atlas), one element per descriptor
        Storage   // storage buffer: an array of elements (instance data), the whole array per descriptor
    };

    UploadBufferVulkan() = default;
    ~UploadBufferVulkan() = default;

    UploadBufferVulkan(const UploadBufferVulkan&) = delete;
    UploadBufferVulkan& operator=(const UploadBufferVulkan&) = delete;

    // Room for elementCount elements of elementSize bytes, per frame in flight. False (logged) on failure; call
    // End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, uint32_t inElementCount, uint32_t inElementSize,
              Kind inKind);
    // The GPU must be done with the buffer (call after vkDeviceWaitIdle).
    void End();

    // Copies one element (elementSize bytes) into a frame's copy.
    void CopyElement(uint32_t frameIndex, uint32_t elementIndex, const void* data);

    // Copies the first count elements from data (packed) into a frame's copy, clamped to the capacity; for instance
    // arrays, as UploadBufferDX::CopyElements.
    void CopyElements(uint32_t frameIndex, const void* data, uint32_t count);

    VkBuffer GetBuffer() const;

    // The descriptor's range: one element for a uniform buffer, the whole array for a storage buffer.
    uint32_t GetDescriptorRange() const;

    // Byte offset of an element in a frame's copy. A dynamic offset for a uniform element, or for element 0 of a
    // storage buffer (the start of the frame's array).
    uint32_t GetOffset(uint32_t frameIndex, uint32_t elementIndex) const;

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    uint8_t* mapped = nullptr;
    Kind kind = Kind::Uniform;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;  // bytes of real data in one element
    uint32_t stride = 0;       // uniform: elementSize rounded up to the alignment; storage: elementSize
    uint32_t frameSize = 0;    // bytes of one frame's copy, rounded up to the alignment
};

}  // namespace ME

#endif  // ME_VULKAN
