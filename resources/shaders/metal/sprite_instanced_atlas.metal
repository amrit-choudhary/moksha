// Simple sprite shader for instanced rendering. Metal port of sprite_instanced_atlas.hlsl: keep them in sync.

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
                    constant CBPerPass& passData [[buffer(0)]],
                    constant TextureAtlasProperties& atlasProps [[buffer(1)]],
                    device const SpriteInstanceData* instanceBuffer [[buffer(3)]]) {
    VertexOut output;

    SpriteInstanceData instanceData = instanceBuffer[instanceID];

    output.position = passData.projectionMatrix * passData.viewMatrix * GetModelMatrix(instanceData) *
                      float4(input.position, 0.0f, 1.0f);
    float2 flippedUV = float2(input.uv.x, 1.0f - input.uv.y);
    output.uv = GetAtlasUV(flippedUV, instanceData.atlasIndex, atlasProps, instanceData.flags);
    output.color = float4(instanceData.color);

    return output;
}

fragment float4 PS(VertexOut input [[stage_in]], texture2d<float> tex [[texture(0)]]) {
    float4 texColor = tex.sample(texSampler, input.uv);
    if (texColor.a < 0.1f) {
        discard_fragment();
    }

    return texColor * input.color;
}
