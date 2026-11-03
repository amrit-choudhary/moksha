// Simple sprite shader for non instanced rendering. WGSL port of sprite_atlas.hlsl: keep them in sync.

#include "structs.wgsl"
#include "helpers.wgsl"

// Bind groups match PSOWebGPU (pso_webgpu.h).
@group(0) @binding(0) var<uniform> passData: CBPerPass;
@group(0) @binding(1) var<uniform> atlasProps: TextureAtlasProperties;
@group(1) @binding(0) var tex: texture_2d<f32>;
@group(1) @binding(1) var texSampler: sampler;
@group(2) @binding(0) var<uniform> spriteData: CBPerSprite;

@vertex
fn VS(input: VertexIn) -> VertexOut {
    var output: VertexOut;

    output.position =
        passData.projectionMatrix * passData.viewMatrix * spriteData.modelMatrix * vec4f(input.position, 0.0, 1.0);
    let flippedUV = vec2f(input.uv.x, 1.0 - input.uv.y);
    output.uv = GetAtlasUV(flippedUV, spriteData.atlasIndex, atlasProps, spriteData.flags);
    output.color = spriteData.color;

    return output;
}

@fragment
fn PS(input: VertexOut) -> @location(0) vec4f {
    let texColor = textureSample(tex, texSampler, input.uv);
    if (texColor.a < 0.1) {
        discard;
    }

    return texColor * input.color;
}
