// Simple sprite shader for non instanced rendering. Metal port of sprite_atlas.hlsl: keep them in sync.

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

// Buffer indices match MetalBindings (pso_metal.h): DX b0, b1, b2 are buffers 0, 1, 2; t0 is texture 0.
vertex VertexOut VS(VertexIn input [[stage_in]],
                    constant CBPerPass& passData [[buffer(0)]],
                    constant TextureAtlasProperties& atlasProps [[buffer(1)]],
                    constant CBPerSprite& spriteData [[buffer(2)]]) {
    VertexOut output;

    output.position =
        passData.projectionMatrix * passData.viewMatrix * spriteData.modelMatrix * float4(input.position, 0.0f, 1.0f);
    float2 flippedUV = float2(input.uv.x, 1.0f - input.uv.y);
    output.uv = GetAtlasUV(flippedUV, spriteData.atlasIndex, atlasProps, spriteData.flags);
    output.color = spriteData.color;

    return output;
}

fragment float4 PS(VertexOut input [[stage_in]], texture2d<float> tex [[texture(0)]]) {
    float4 texColor = tex.sample(texSampler, input.uv);
    if (texColor.a < 0.1f) {
        discard_fragment();
    }

    return texColor * input.color;
}
