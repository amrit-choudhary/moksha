#version 450
#extension GL_GOOGLE_include_directive : require

// UI sprite shader for instanced rendering, vertex stage. GLSL port of ui_sprite.hlsl: keep them in sync.

#include "structs.glsl"
#include "helpers.glsl"

// Declared directly, not as a struct: a std140 struct rounds up to 16 bytes, larger than the 4-byte C++ constants.
layout(std140, set = 0, binding = 0) uniform PerPassBlock {
    uint screenDimension;  // packed: high 16 bits width, low 16 bits height
};

layout(std430, set = 1, binding = 0) readonly buffer InstanceBlock {
    UISpriteInstanceData instances[];
};

layout(std140, set = 1, binding = 1) uniform AtlasBlock {
    TextureAtlasProperties atlasProps;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main() {
    UISpriteInstanceData instData = instances[gl_InstanceIndex];
    gl_Position = GetUIPosition(instData.modelMatrix * vec4(inPosition, 0.0, 1.0), screenDimension);
    vec2 flippedUV = vec2(inUV.x, 1.0 - inUV.y);
    outUV = GetAtlasUV(flippedUV, instData.atlasIndex, atlasProps, instData.flags);
    outColor = instData.color;
}
