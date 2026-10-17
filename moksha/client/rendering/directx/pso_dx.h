#pragma once
#ifdef ME_DX12

/**
 * DirectX's specific pipeline state object (PSO) class.
 */

#include <d3d12.h>

#include "moksha/shared/math/vec16.h"
#include "shader_dx.h"

namespace ME {

/** Constant buffer structure for shaders. */
class CBPerPass {
   public:
    ME::Vec16 viewMatrix;
    ME::Vec16 projectionMatrix;
    ME::LightDataAmbient ambientLightData;
    ME::LightDataDirectional directionalLightData;
};

/** Constant buffer structure for UI Text shaders. */
class CBPerPassUIText {
   public:
    // Packed screen dimension: high 16 bits = width, low 16 bits = height.
    uint32_t screenDimension;
};

/** Constant buffer structure for UI Sprite shaders. */
class CBPerPassUISprite {
   public:
    // Packed screen dimension: high 16 bits = width, low 16 bits = height.
    uint32_t screenDimension;
};

/** Constant buffer structure for shaders. */
class CBPerObject {
   public:
    ME::Vec16 modelMatrix;
};

class PSODirectX {
   public:
    PSODirectX();
    ~PSODirectX();

    /** Create PSO for 3D rendering */
    static ID3D12PipelineState* CreatePSO3D(ID3D12Device* device, const char* shaderName,
                                            ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D rendering */
    static ID3D12PipelineState* CreatePSO2D(ID3D12Device* device, const char* shaderName,
                                            ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D rendering with instancing. */
    static ID3D12PipelineState* CreatePSO2DInstanced(ID3D12Device* device, const char* shaderName,
                                                     ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D rendering with instancing and texture atlases. */
    static ID3D12PipelineState* CreatePSO2DInstancedAtlas(ID3D12Device* device, const char* shaderName,
                                                          ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D rendering with no instancing and texture atlases. */
    static ID3D12PipelineState* CreatePSO2DAtlas(ID3D12Device* device, const char* shaderName,
                                                 ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D UI Text rendering. */
    static ID3D12PipelineState* CreatePSO2DUIText(ID3D12Device* device, const char* shaderName,
                                                  ID3D12RootSignature* rootSignature);

    /** Create PSO for 2D UI Sprite rendering. */
    static ID3D12PipelineState* CreatePSO2DUISprite(ID3D12Device* device, const char* shaderName,
                                                    ID3D12RootSignature* rootSignature);

   private:
};

}  // namespace ME

#endif  // ME_DX12
