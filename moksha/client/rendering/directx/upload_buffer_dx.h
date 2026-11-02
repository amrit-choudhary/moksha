#pragma once
#ifdef ME_DX12

/**
 * DirectX's specific upload buffer class.
 * Used for uploading data to the GPU like constant buffers.
 * This creates an upload heap buffer and maps it to a CPU pointer to be able to write to it.
 * Holds one copy of the data per frame in flight, so writing one frame's copy never touches what the GPU is reading.
 */

#include <d3d12.h>

#include <cstddef>
#include <cstdint>

namespace ME {

class UploadBufferDX {
   public:
    UploadBufferDX(ID3D12Device* device, bool isConstantBuffer, uint32_t elementCount, uint32_t elementSize);
    ~UploadBufferDX();

    UploadBufferDX(const UploadBufferDX&) = delete;
    UploadBufferDX& operator=(const UploadBufferDX&) = delete;

    ID3D12Resource* GetResource() const;
    uint32_t GetElementSize() const;

    /**
     * Copies an array of data to a frame's copy in the upload buffer.
     * Data should be of the same size as elementSize * elementCount.
     */
    void CopyData(uint32_t frameIndex, const void* data);

    /**
     * Copies the first count elements from data to a frame's copy, packed at the element size.
     * Count is clamped to the capacity.
     */
    void CopyElements(uint32_t frameIndex, const void* data, uint32_t count);

   private:
    // Byte offset of a frame's copy from the start of the buffer.
    size_t GetFrameOffset(uint32_t frameIndex) const;

    bool isConstantBuffer = false;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;
    ID3D12Resource* uploadBuffer = nullptr;
    uint8_t* mappedData = nullptr;
};

}  // namespace ME

#endif  // ME_DX12
