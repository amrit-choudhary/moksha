#version 450
#extension GL_GOOGLE_include_directive : require

// Simple sprite shader for instanced rendering, vertex stage. GLSL port of sprite_instanced_atlas.hlsl: keep them in sync.

#include "structs.glsl"
#include "helpers.glsl"

layout(std140, set = 0, binding = 0) uniform PerPassBlock {
    CBPerPass passData;
};

layout(std430, set = 1, binding = 0) readonly buffer InstanceBlock {
    SpriteInstanceData instances[];
};

layout(std140, set = 1, binding = 1) uniform AtlasBlock {
    TextureAtlasProperties atlasProps;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main() {
    SpriteInstanceData instData = instances[gl_InstanceIndex];
    gl_Position = passData.projectionMatrix * passData.viewMatrix * instData.modelMatrix * vec4(inPosition, 0.0, 1.0);
    vec2 flippedUV = vec2(inUV.x, 1.0 - inUV.y);
    outUV = GetAtlasUV(flippedUV, instData.atlasIndex, atlasProps, instData.flags);
    outColor = instData.color;
}
