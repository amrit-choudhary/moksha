#pragma once
#ifdef ME_METAL

/**
 * Metal's pipeline state objects, the counterpart of PSODirectX: same shaders, vertex layout, blending and back
 * buffer format. Metal has no root signature: shaders read the fixed argument table indices in MetalBindings. Depth
 * is a separate state set on the encoder (DX bakes it into the PSO), as are cull mode and winding.
 */

#include <cstdint>

namespace MTL {
class DepthStencilState;
class Device;
class RenderPipelineState;
}  // namespace MTL

namespace MTL4 {
class Compiler;
}  // namespace MTL4

namespace ME {

// Argument table indices, matching the [[buffer(n)]] / [[texture(n)]] attributes in shaders/metal/.
namespace MetalBindings {
constexpr uint32_t PerPassBuffer = 0;    // DX b0: per-pass constants
constexpr uint32_t AtlasBuffer = 1;      // DX b1: texture atlas properties
constexpr uint32_t PerObjectBuffer = 2;  // DX b2: per-sprite constants
constexpr uint32_t InstanceBuffer = 3;   // DX t1: instance data
constexpr uint32_t VertexBuffer = 4;     // DX input assembler: the quad's vertices
constexpr uint32_t BufferCount = 5;
constexpr uint32_t Texture = 0;  // DX t0
constexpr uint32_t TextureCount = 1;
}  // namespace MetalBindings

class PSOMetal {
   public:
    // Sprites with texture atlases, no instancing (shaders/metal/sprite_atlas.metal). Null on failure (logged).
    static MTL::RenderPipelineState* CreatePSO2DAtlas(MTL4::Compiler* compiler, const char* shaderName);

    // Instanced sprites with texture atlases (shaders/metal/sprite_instanced_atlas.metal). Null on failure (logged).
    static MTL::RenderPipelineState* CreatePSO2DInstancedAtlas(MTL4::Compiler* compiler, const char* shaderName);

    // UI sprites, instanced (shaders/metal/ui_sprite.metal). Null on failure (logged).
    static MTL::RenderPipelineState* CreatePSO2DUISprite(MTL4::Compiler* compiler, const char* shaderName);

    // UI text, instanced (shaders/metal/ui_text.metal). Null on failure (logged).
    static MTL::RenderPipelineState* CreatePSO2DUIText(MTL4::Compiler* compiler, const char* shaderName);

    // Depth test less with writes: the DX default, used by the sprite passes.
    static MTL::DepthStencilState* CreateDepthStateDefault(MTL::Device* device);

    // No depth test or writes: the UI passes.
    static MTL::DepthStencilState* CreateDepthStateDisabled(MTL::Device* device);
};

}  // namespace ME

#endif  // ME_METAL
