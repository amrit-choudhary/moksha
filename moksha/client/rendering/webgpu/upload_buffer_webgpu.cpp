#ifdef ME_WEBGPU

#include "upload_buffer_webgpu.h"

#include <cstring>

ME::UploadBufferWebGPU::UploadBufferWebGPU(WGPUDevice device, WGPUQueue inQueue, bool isConstantBuffer,
                                           uint32_t inElementCount, uint32_t inElementSize) {
    queue = inQueue;
    elementCount = inElementCount;
    elementSize = inElementSize;
    stride = isConstantBuffer ? ((inElementSize + 255) & ~255u) : inElementSize;

    WGPUBufferDescriptor descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
    descriptor.usage = WGPUBufferUsage_CopyDst | (isConstantBuffer ? WGPUBufferUsage_Uniform : WGPUBufferUsage_Storage);
    descriptor.size = static_cast<uint64_t>(stride) * elementCount;
    buffer = wgpuDeviceCreateBuffer(device, &descriptor);

    if (isConstantBuffer) {
        cpuCopy = new uint8_t[static_cast<size_t>(stride) * elementCount]();
    }
}

ME::UploadBufferWebGPU::~UploadBufferWebGPU() {
    delete[] cpuCopy;
    if (buffer != nullptr) {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }
}

void ME::UploadBufferWebGPU::SetElement(uint32_t index, const void* data) {
    if (cpuCopy != nullptr && data != nullptr && index < elementCount) {
        std::memcpy(cpuCopy + static_cast<size_t>(stride) * index, data, elementSize);
    }
}

void ME::UploadBufferWebGPU::Upload() {
    if (cpuCopy != nullptr) {
        wgpuQueueWriteBuffer(queue, buffer, 0, cpuCopy, static_cast<size_t>(stride) * elementCount);
    }
}

void ME::UploadBufferWebGPU::WriteElements(const void* data, uint32_t count) {
    const uint32_t writeCount = count < elementCount ? count : elementCount;
    if (data != nullptr && writeCount != 0) {
        wgpuQueueWriteBuffer(queue, buffer, 0, data, static_cast<size_t>(stride) * writeCount);
    }
}

WGPUBuffer ME::UploadBufferWebGPU::GetBuffer() const {
    return buffer;
}

uint32_t ME::UploadBufferWebGPU::GetElementSize() const {
    return elementSize;
}

uint32_t ME::UploadBufferWebGPU::GetOffset(uint32_t index) const {
    return stride * index;
}

#endif  // ME_WEBGPU
