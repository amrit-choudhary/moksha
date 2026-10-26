#pragma once
#ifdef ME_METAL

/**
 * Metal's upload buffer, the counterpart of UploadBufferDX: a CPU-writable buffer (constant or instance data) the
 * GPU reads directly. Bound by GPU address in an argument table; the owner adds GetBuffer() to a residency set.
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

    // GPU address of an element, for an argument table binding.
    uint64_t GetGPUAddress(uint32_t elementIndex = 0) const;

    // Copies elementSize * elementCount bytes from data.
    void CopyData(const void* data);

    // Copies one element's data to elementIndex.
    void CopyData(uint32_t elementIndex, const void* data);

   private:
    MTL::Buffer* buffer = nullptr;
    uint8_t* mappedData = nullptr;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;
};

}  // namespace ME

#endif  // ME_METAL
