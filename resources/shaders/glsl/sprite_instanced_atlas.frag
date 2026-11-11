#version 450

// Simple sprite shader for instanced rendering, vertex stage. GLSL port of sprite_instanced_atlas.hlsl: keep them in sync.

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D tex;

void main() {
    vec4 texColor = texture(tex, inUV);
    if (texColor.a < 0.1) {
        discard;
    }
    outColor = texColor * inColor;
}
