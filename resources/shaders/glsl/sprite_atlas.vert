#version 450

// Simple sprite shader for non instanced rendering, vertex stage. GLSL port of sprite_atlas.hlsl: keep them in sync.
// TODO: not ported yet: camera and sprite constants, atlas UVs.

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outUV = inUV;
}
