#ifdef ME_METAL

#include "upload_buffer_metal.h"

#include <Metal/Metal.hpp>
#include <cstring>

ME::UploadBufferMetal::UploadBufferMetal(MTL::Device* device, bool isConstantBuffer, uint32_t inElementCount,
                                         uint32_t inElementSize) {
    elementCount = inElementCount;
    elementSize = inElementSize;

    // Constant elements keep UploadBufferDX's 256-byte stride, which also meets Metal's constant offset alignment.
    if (isConstantBuffer) {
        elementSize = (elementSize + 255) & ~255u;
    }

    // Shared storage: CPU and GPU see the same memory on Apple silicon, so there is no upload copy.
    buffer = device->newBuffer(static_cast<NS::UInteger>(elementSize) * elementCount, MTL::ResourceStorageModeShared);
    if (buffer != nullptr) {
        mappedData = static_cast<uint8_t*>(buffer->contents());
    }
}

ME::UploadBufferMetal::~UploadBufferMetal() {
    if (buffer != nullptr) {
        buffer->release();
        buffer = nullptr;
    }
    mappedData = nullptr;
}

MTL::Buffer* ME::UploadBufferMetal::GetBuffer() const {
    return buffer;
}

uint32_t ME::UploadBufferMetal::GetElementSize() const {
    return elementSize;
}

uint64_t ME::UploadBufferMetal::GetGPUAddress(uint32_t elementIndex) const {
    return buffer != nullptr ? buffer->gpuAddress() + static_cast<uint64_t>(elementIndex) * elementSize : 0;
}

void ME::UploadBufferMetal::CopyData(const void* data) {
    if (mappedData != nullptr && data != nullptr) {
        std::memcpy(mappedData, data, static_cast<size_t>(elementSize) * elementCount);
    }
}

void ME::UploadBufferMetal::CopyData(uint32_t elementIndex, const void* data) {
    if (mappedData != nullptr && data != nullptr && elementIndex < elementCount) {
        std::memcpy(mappedData + static_cast<size_t>(elementIndex) * elementSize, data, elementSize);
    }
}

#endif  // ME_METAL
