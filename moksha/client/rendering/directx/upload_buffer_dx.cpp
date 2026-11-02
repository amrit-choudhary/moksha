#ifdef ME_DX12

#include "upload_buffer_dx.h"

#include "d3dx12.h"
#include "moksha/shared/misc/engine_limits.h"

ME::UploadBufferDX::UploadBufferDX(ID3D12Device* device, bool isConstantBuffer, uint32_t elementCount,
                                   uint32_t elementSize) {
    this->isConstantBuffer = isConstantBuffer;
    this->elementCount = elementCount;
    this->elementSize = elementSize;

    if (isConstantBuffer) {
        // Constant buffers must be a multiple of 256 bytes.
        this->elementSize = (elementSize + 255) & ~255;
    }

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(this->elementSize) *
                                                                     this->elementCount * Constants::FramesInFlight);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, IID_PPV_ARGS(&uploadBuffer));

    uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
}

ME::UploadBufferDX::~UploadBufferDX() {
    if (uploadBuffer != nullptr) {
        uploadBuffer->Unmap(0, nullptr);
        uploadBuffer->Release();
        uploadBuffer = nullptr;
    }
}

ID3D12Resource* ME::UploadBufferDX::GetResource() const {
    return uploadBuffer;
}

uint32_t ME::UploadBufferDX::GetElementSize() const {
    return elementSize;
}

void ME::UploadBufferDX::CopyData(uint32_t frameIndex, const void* data) {
    if (mappedData && data && frameIndex < Constants::FramesInFlight) {
        memcpy(mappedData + GetFrameOffset(frameIndex), data, static_cast<size_t>(elementSize) * elementCount);
    }
}

void ME::UploadBufferDX::CopyElements(uint32_t frameIndex, const void* data, uint32_t count) {
    if (mappedData && data && frameIndex < Constants::FramesInFlight) {
        const uint32_t copyCount = count < elementCount ? count : elementCount;
        memcpy(mappedData + GetFrameOffset(frameIndex), data, static_cast<size_t>(elementSize) * copyCount);
    }
}

size_t ME::UploadBufferDX::GetFrameOffset(uint32_t frameIndex) const {
    return static_cast<size_t>(frameIndex) * elementSize * elementCount;
}

#endif  // ME_DX12
