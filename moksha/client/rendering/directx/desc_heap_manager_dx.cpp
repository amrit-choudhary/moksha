#ifdef ME_WIN

#include "desc_heap_manager_dx.h"

#endif  // ME_WIN

ME::DescHeapManagerDX::DescHeapManagerDX(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap) {
    this->device = device;
    this->descriptorHeap = descriptorHeap;
    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    currentIndex = 0;
}

uint32_t ME::DescHeapManagerDX::CreateCBV(ID3D12Resource* resource, const uint32_t sizeInBytes) {
    D3D12_CONSTANT_BUFFER_VIEW_DESC descDesc{};
    descDesc.BufferLocation = resource->GetGPUVirtualAddress();
    descDesc.SizeInBytes = sizeInBytes;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += currentIndex * descriptorSize;
    device->CreateConstantBufferView(&descDesc, cpuHandle);

    ++currentIndex;
    return (currentIndex - 1);
}

uint32_t ME::DescHeapManagerDX::CreateSRVTexture(ID3D12Resource* resource) {
    D3D12_SHADER_RESOURCE_VIEW_DESC descDesc{};
    descDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    descDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descDesc.Texture2D.MostDetailedMip = 0;
    descDesc.Texture2D.MipLevels = -1;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += currentIndex * descriptorSize;
    device->CreateShaderResourceView(resource, &descDesc, cpuHandle);

    ++currentIndex;
    return (currentIndex - 1);
}

uint32_t ME::DescHeapManagerDX::CreateSRVInstanceData(ID3D12Resource* resource, const uint32_t elementSize,
                                                      const uint32_t elementCount) {
    D3D12_SHADER_RESOURCE_VIEW_DESC descDesc{};
    descDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descDesc.Format = DXGI_FORMAT_UNKNOWN;
    descDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    descDesc.Buffer.FirstElement = 0;
    descDesc.Buffer.NumElements = elementCount;
    descDesc.Buffer.StructureByteStride = elementSize;
    descDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += currentIndex * descriptorSize;
    device->CreateShaderResourceView(resource, &descDesc, cpuHandle);

    ++currentIndex;
    return (currentIndex - 1);
}

uint32_t ME::DescHeapManagerDX::GetDescriptorSize() const {
    return descriptorSize;
}

uint32_t ME::DescHeapManagerDX::GetCurrentIndex() const {
    return currentIndex;
}

uint32_t ME::DescHeapManagerDX::GetDescriptorCount() const {
    if (currentIndex == 0) {
        return 0;
    } else {
        return currentIndex;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE ME::DescHeapManagerDX::GetCPUDescriptorHandleForIndex(const uint32_t index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += index * descriptorSize;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ME::DescHeapManagerDX::GetGPUDescriptorHandleForIndex(const uint32_t index) const {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += index * descriptorSize;
    return handle;
}
