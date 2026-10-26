// UI text shader for instanced rendering. Metal port of ui_text.hlsl: keep them in sync.

#include "structs.metal"
#include "helpers.metal"

// Using clamped point sampling for pixel art style atlases (the DX static sampler s1).
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

// Buffer indices match MetalBindings (pso_metal.h): DX b0, b1 are buffers 0, 1; the t1 instance buffer is buffer 3;
// t0 is texture 0.
vertex VertexOut VS(VertexIn input [[stage_in]],
                    uint instanceID [[instance_id]],
                    constant CBPerPassUIText& passData [[buffer(0)]],
                    constant TextureAtlasProperties& atlasProps [[buffer(1)]],
                    device const TextInstanceData* instanceBuffer [[buffer(3)]]) {
    VertexOut output;

    TextInstanceData instanceData = instanceBuffer[instanceID];

    float4 outputPos = GetModelMatrix(instanceData) * float4(input.position, 0.0f, 1.0f);
    // Mapping to NDC space for UI rendering. [-1, 1] range.
    // Screen dimensions are packed in a single uint32_t.
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
