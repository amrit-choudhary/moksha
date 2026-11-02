#ifdef ME_DX12

#include "desc_heap_manager_dx.h"

#endif  // ME_DX12

#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"

ME::DescHeapManagerDX::DescHeapManagerDX(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap) {
    this->device = device;
    this->descriptorHeap = descriptorHeap;
    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    currentIndex = 0;
}

bool ME::DescHeapManagerDX::HasFreeDescriptors(const uint32_t count) const {
    if (currentIndex + count <= Constants::MaxDescriptorsOnDescriptorHeap) {
        return true;
    }
    ME::LogError("DescHeapManagerDX: the descriptor heap is full (MaxDescriptorsOnDescriptorHeap)");
    return false;
}

uint32_t ME::DescHeapManagerDX::CreateFrameCBVs(ID3D12Resource* resource, const uint32_t sizeInBytes) {
    if (!HasFreeDescriptors(Constants::FramesInFlight)) {
        return 0;
    }

    const uint32_t firstIndex = currentIndex;
    for (uint32_t frame = 0; frame < Constants::FramesInFlight; ++frame) {
        D3D12_CONSTANT_BUFFER_VIEW_DESC descDesc{};
        descDesc.BufferLocation = resource->GetGPUVirtualAddress() + static_cast<uint64_t>(frame) * sizeInBytes;
        descDesc.SizeInBytes = sizeInBytes;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += currentIndex * descriptorSize;
        device->CreateConstantBufferView(&descDesc, cpuHandle);

        ++currentIndex;
    }
    return firstIndex;
}

uint32_t ME::DescHeapManagerDX::CreateSRVTexture(ID3D12Resource* resource) {
    if (!HasFreeDescriptors(1)) {
        return 0;
    }

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

uint32_t ME::DescHeapManagerDX::CreateFrameSRVsInstanceData(ID3D12Resource* resource, const uint32_t elementSize,
                                                            const uint32_t elementCount) {
    if (!HasFreeDescriptors(Constants::FramesInFlight)) {
        return 0;
    }

    const uint32_t firstIndex = currentIndex;
    for (uint32_t frame = 0; frame < Constants::FramesInFlight; ++frame) {
        D3D12_SHADER_RESOURCE_VIEW_DESC descDesc{};
        descDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        descDesc.Format = DXGI_FORMAT_UNKNOWN;
        descDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        descDesc.Buffer.FirstElement = static_cast<uint64_t>(frame) * elementCount;
        descDesc.Buffer.NumElements = elementCount;
        descDesc.Buffer.StructureByteStride = elementSize;
        descDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += currentIndex * descriptorSize;
        device->CreateShaderResourceView(resource, &descDesc, cpuHandle);

        ++currentIndex;
    }
    return firstIndex;
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
