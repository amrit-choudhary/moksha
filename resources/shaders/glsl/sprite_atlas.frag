#version 450

// Simple sprite shader for non instanced rendering, fragment stage. GLSL port of sprite_atlas.hlsl: keep them in sync.
// TODO: not ported yet: the sprite color and the alpha discard.

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D tex;

void main() {
    outColor = texture(tex, inUV);
}
