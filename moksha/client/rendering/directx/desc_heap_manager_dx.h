#pragma once
#ifdef ME_WIN

/**
 * A utility class for managing descriptor heap in DirectX 12.
 * Mainly used to create desc views for elements like textures and buffers in the scene.
 * This is for CBV_SRV_UAV descriptor heap.
 */

#include <d3d12.h>

#include <cstdint>

#include "d3dx12.h"

namespace ME {

class DescHeapManagerDX {
   public:
    DescHeapManagerDX() = delete;
    DescHeapManagerDX(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap);
    ~DescHeapManagerDX() = default;

    /**
     * Creates a constant buffer view for the given resource and returns the index in the descriptor heap.
     */
    uint32_t CreateCBV(ID3D12Resource* resource, const uint32_t sizeInBytes);

    /**
     * Creates a shader resource view for the given texture resource and returns the index in the descriptor heap.
     * Texture stores the index for later use of getting GPU and CPU handles.
     */
    uint32_t CreateSRVTexture(ID3D12Resource* resource);

    /**
     * Creates a shader resource view for the given instance data buffer and returns the index in the descriptor heap.
     * Used for instance data buffers.
     */
    uint32_t CreateSRVInstanceData(ID3D12Resource* resource, const uint32_t elementSize, const uint32_t elementCount);

    uint32_t GetDescriptorSize() const;

    uint32_t GetCurrentIndex() const;

    /** Returns the total number of descriptors allocated in the heap. */
    uint32_t GetDescriptorCount() const;

    /** Returns the CPU descriptor handle for the given index. */
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForIndex(const uint32_t index) const;

    /** Returns the GPU descriptor handle for the given index. */
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForIndex(const uint32_t index) const;

   private:
    ID3D12Device* device = nullptr;
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    // Size of a single descriptor in the heap.
    uint32_t descriptorSize = 0;
    // Current index to allocate descriptor from. If 2 descriptors are created already, this will be 3.
    uint32_t currentIndex = 0;
};

}  // namespace ME

#endif  // ME_WIN
