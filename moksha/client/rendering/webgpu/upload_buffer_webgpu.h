#pragma once
#ifdef ME_WEBGPU

/**
 * A GPU buffer the CPU writes every frame through the queue, which copies the data, so one copy of it is enough.
 * Constant buffers hold elements at a 256-byte stride, the dynamic offset alignment, and keep a CPU copy to upload all
 * at once. Instance buffers are written straight from the caller's tightly packed array.
 */

#include <webgpu/webgpu.h>

#include <cstdint>

namespace ME {

class UploadBufferWebGPU {
   public:
    UploadBufferWebGPU(WGPUDevice device, WGPUQueue inQueue, bool isConstantBuffer, uint32_t inElementCount,
                       uint32_t inElementSize);
    ~UploadBufferWebGPU();

    UploadBufferWebGPU(const UploadBufferWebGPU&) = delete;
    UploadBufferWebGPU& operator=(const UploadBufferWebGPU&) = delete;

    // Constant buffers: sets one element in the CPU copy. Upload sends the whole copy to the GPU.
    void SetElement(uint32_t index, const void* data);
    void Upload();

    // Instance buffers: sends the first count elements to the GPU, clamped to the buffer's capacity.
    void WriteElements(const void* data, uint32_t count);

    WGPUBuffer GetBuffer() const;

    // Size of one element for a bind group, and an element's dynamic offset.
    uint32_t GetElementSize() const;
    uint32_t GetOffset(uint32_t index) const;

   private:
    WGPUBuffer buffer = nullptr;
    WGPUQueue queue = nullptr;
    uint8_t* cpuCopy = nullptr;
    uint32_t elementCount = 0;
    uint32_t elementSize = 0;
    uint32_t stride = 0;
};

}  // namespace ME

#endif  // ME_WEBGPU
