#ifdef ME_DX12

#include "upload_buffer_dx.h"

#include "d3dx12.h"

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
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(this->elementSize * this->elementCount);
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

uint8_t* ME::UploadBufferDX::GetMappedData() const {
    return mappedData;
}

uint32_t ME::UploadBufferDX::GetElementSize() const {
    return elementSize;
}

void ME::UploadBufferDX::CopyData(const void* data) {
    if (mappedData && data) {
        memcpy(mappedData, data, elementSize * elementCount);
    }
}

void ME::UploadBufferDX::CopyData(uint32_t elementIndex, const void* data) {
    if (mappedData && data && elementIndex < elementCount) {
        memcpy(&mappedData[elementIndex * elementSize], data, elementSize);
    }
}

#endif  // ME_DX12
