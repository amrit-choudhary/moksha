#pragma once
#ifdef ME_WIN

/**
 * DirectX's specific utility functions and definitions.
 */

#include <d3d12.h>

#include <cstdint>

#include "d3dx12.h"

namespace ME {
namespace UtilsDirectX {

/**
 * Creates a default buffer resource in DirectX 12. Used for vertex and index buffers.
 */
ID3D12Resource* CreateDefaultBufferResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
                                            const void* initData, uint32_t byteSize, ID3D12Resource** uploadBuffer);

}  // namespace UtilsDirectX
}  // namespace ME

#endif  // ME_WIN
