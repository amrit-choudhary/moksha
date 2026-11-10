#version 450
#extension GL_GOOGLE_include_directive : require

// Simple sprite shader for non instanced rendering, vertex stage. GLSL port of sprite_atlas.hlsl: keep them in sync.

#include "structs.glsl"
#include "helpers.glsl"

layout(std140, set = 0, binding = 0) uniform PerPassBlock {
    CBPerPass passData;
};

layout(std140, set = 1, binding = 0) uniform PerSpriteBlock {
    CBPerSprite spriteData;
};

layout(std140, set = 1, binding = 1) uniform AtlasBlock {
    TextureAtlasProperties atlasProps;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main() {
    gl_Position = passData.projectionMatrix * passData.viewMatrix * spriteData.modelMatrix * vec4(inPosition, 0.0, 1.0);
    vec2 flippedUV = vec2(inUV.x, 1.0 - inUV.y);
    outUV = GetAtlasUV(flippedUV, spriteData.atlasIndex, atlasProps, spriteData.flags);
    outColor = spriteData.color;
}
