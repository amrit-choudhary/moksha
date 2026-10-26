// UI text shader for instanced rendering. Metal port of shaders/hlsl/ui_text.hlsl: keep them in sync.
// Self-contained (compiled from source at startup, so no #include): the structs and the atlas helper are repeated.

#include <metal_stdlib>
using namespace metal;

// Layouts match the C++ classes byte for byte (rendering/shared/texture.h, sprite_renderer.h, text_renderer.h,
// shader_constants.h).
struct TextureAtlasProperties {
    uint tileSizeX;
    uint tileSizeY;
    uint padding;
    uint numTextures;
    uint numTilesX;
    uint numTilesY;
    uint width;
    uint height;
    uint paddingType;
    uint gpuPadding[3];
};

// Screen dimension packed in one uint: high 16 bits width, low 16 bits height.
struct CBPerPassUI {
    uint screenDimension;
};

// The instance data is 88 bytes with 4-byte alignment; packed columns keep that stride (a float4x4 member would pad
// it to 96). The matrix columns are column-major, from Mat4::GetDataForShader.
struct UIInstanceData {
    packed_float4 modelColumn0;
    packed_float4 modelColumn1;
    packed_float4 modelColumn2;
    packed_float4 modelColumn3;
    packed_float4 color;
    uint atlasIndex;
    uint flags;
};

// Atlas UV of a tile, from quad UVs (0,0)-(1,1). Atlas index counts from the top left; flags bit 0/1 flip H/V.
float2 GetAtlasUV(float2 uv, uint atlasIndex, constant TextureAtlasProperties& atlasProps, uint flags) {
    float tileW = (float)atlasProps.tileSizeX;
    float tileH = (float)atlasProps.tileSizeY;
    float pad = (float)atlasProps.padding;

    float strideX = tileW + pad;
    float strideY = tileH + pad;

    uint col = atlasIndex % atlasProps.numTilesX;
    uint row = atlasIndex / atlasProps.numTilesX;

    float originX = col * strideX;
    float originY = row * strideY;

    float2 localPx = uv * float2(tileW, tileH);

    bool flipH = (flags & 0x1u) != 0u;
    bool flipV = (flags & 0x2u) != 0u;
    if (flipH) localPx.x = tileW - localPx.x;
    if (flipV) localPx.y = tileH - localPx.y;

    float2 atlasPx = float2(originX, originY) + localPx;
    return atlasPx / float2((float)atlasProps.width, (float)atlasProps.height);
}

// Clamped point sampling for pixel art atlases: the DX static sampler s1.
constexpr sampler texSampler(filter::nearest, mip_filter::nearest, address::clamp_to_edge);

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 uv;
    float4 color;
};

// Buffer indices match RendererMetal's argument table: DX b0, b1 are buffers 0, 1; the t1 instance buffer is buffer 3;
// t0 is texture 0.
vertex VertexOut VS(VertexIn input [[stage_in]],
                    uint instanceID [[instance_id]],
                    constant CBPerPassUI& passData [[buffer(0)]],
                    constant TextureAtlasProperties& atlasProps [[buffer(1)]],
                    device const UIInstanceData* instanceBuffer [[buffer(3)]]) {
    VertexOut output;

    UIInstanceData instanceData = instanceBuffer[instanceID];
    float4x4 modelMatrix = float4x4(float4(instanceData.modelColumn0), float4(instanceData.modelColumn1),
                                    float4(instanceData.modelColumn2), float4(instanceData.modelColumn3));

    float4 outputPos = modelMatrix * float4(input.position, 0.0f, 1.0f);
    // Mapping to NDC space for UI rendering. [-1, 1] range.
    uint screenWidth = (passData.screenDimension >> 16) & 0xFFFF;
    uint screenHeight = passData.screenDimension & 0xFFFF;
    outputPos.x = (outputPos.x / screenWidth) * 2.0f;
    outputPos.y = (outputPos.y / screenHeight) * 2.0f;
    output.position = outputPos;
    float2 flippedUV = float2(input.uv.x, 1.0f - input.uv.y);
    output.uv = GetAtlasUV(flippedUV, instanceData.atlasIndex, atlasProps, instanceData.flags);
    output.color = float4(instanceData.color);

    return output;
}

// The font texture only masks the glyph; the colour is the instance's, as in ui_text.hlsl.
fragment float4 PS(VertexOut input [[stage_in]], texture2d<float> tex [[texture(0)]]) {
    float4 texColor = tex.sample(texSampler, input.uv);
    if (texColor.a < 0.1f) {
        discard_fragment();
    }

    return input.color;
}
