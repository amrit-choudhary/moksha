#pragma once
#ifdef ME_METAL

/**
 * Metal's upload buffer, the counterpart of UploadBufferDX: a CPU-writable buffer (constant or instance data) the
 * GPU reads directly. Bound by GPU address in an argument table; the owner adds GetBuffer() to a residency set.
 * Holds one copy of the data per frame in flight, so writing one frame's copy never touches what the GPU is reading.
 */

#include <cstdint>

namespace MTL {
class Buffer;
class Device;
}  // namespace MTL

namespace ME {

class UploadBufferMetal {
   public:
    UploadBufferMetal(MTL::Device* device, bool isConstantBuffer, uint32_t elementCount, uint32_t elementSize);
    ~UploadBufferMetal();

    UploadBufferMetal(const UploadBufferMetal&) = delete;
    UploadBufferMetal& operator=(const UploadBufferMetal&) = delete;

    MTL::Buffer* GetBuffer() const;
    uint32_t GetElementSize() const;

    // GPU address of a frame's copy, for an argument table binding.
    uint64_t GetGPUAddress(uint32_t frameIndex) const;

    // Copies elementSize * elementCount bytes from data into a frame's copy.
    void CopyData(uint32_t frameIndex, const void* data);

    // Copies the first count elements from data into a frame's copy; count is clamped to the capacity.
    void CopyElements(uint32_t frameIndex, const void* data, uint32_t count);

   private:
    // Byte offset of a frame's copy from the start of the buffer.
    uint64_t GetFrameOffset(uint32_t frameIndex) const;

    MTL::Buffer* buffer = nullptr;
    uint8_t* mappedData = nullptr;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;
};

}  // namespace ME

#endif  // ME_METAL
