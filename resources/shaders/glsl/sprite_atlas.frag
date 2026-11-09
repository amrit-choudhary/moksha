#version 450

// Simple sprite shader for non instanced rendering, fragment stage. GLSL port of sprite_atlas.hlsl: keep them in sync.
// TODO: not ported yet: draws the UVs as a gradient instead of the texture.

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inUV, 0.5, 1.0);
}
