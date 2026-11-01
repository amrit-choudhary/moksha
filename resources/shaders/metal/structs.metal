// This contains metal version of game side structs used for rendering: the counterpart of structs.hlsl.
// Include this file in your metal shaders to get access to these structs. Layouts match the C++ classes byte for byte
// (rendering/shared/light.h, texture.h, sprite_renderer.h, text_renderer.h, shader_constants.h).

#pragma once

#include <metal_stdlib>
using namespace metal;

// Ambient light data.
struct LightDataAmbient {
    float4 color;
    float intensity;
    float padding[3];
};

// Directional light data. packed_float3 keeps the C++ 12-byte Vec3 (a float3 would take 16).
struct LightDataDirectional {
    packed_float3 direction;
    float padding1;
    float intensity;
    float padding2[3];
    float4 color;
};

// Per Instance Data for Sprite Renderer
struct SpriteInstanceData {
    float4x4 modelMatrix;
    float4 color;
    uint atlasIndex;
    uint flags;
    uint2 padding;
};

// Per Instance Data for UI Sprite Renderer
struct UISpriteInstanceData {
    float4x4 modelMatrix;
    float4 color;
    uint atlasIndex;
    uint flags;
    uint2 padding;
};

// Per Instance Data for Text Renderer
struct TextInstanceData {
    float4x4 modelMatrix;
    float4 color;
    uint atlasIndex;
    uint flags;
    uint2 padding;
};

// Texture Atlas Properties
struct TextureAtlasProperties {
    uint tileSizeX;
    uint tileSizeY;
    uint padding;
    uint numTextures;
    uint numTilesX;
    uint numTilesY;
    uint width;
    uint height;
    uint paddingType;
    uint gpuPadding[3];
};

// Per Sprite Constant Buffer
struct CBPerSprite {
    float4x4 modelMatrix;
    float4 color;
    uint atlasIndex;
    uint flags;
    uint2 padding;
};

// Per Pass Constant Buffer
struct CBPerPass {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    LightDataAmbient ambientLightData;
    LightDataDirectional directionalLightData;
};

// Per Pass Constant Buffer for UI Text
struct CBPerPassUIText {
    uint screenDimension;  // Packed: high 16 bits width, low 16 bits height.
};

// Per Pass Constant Buffer for UI Sprite
struct CBPerPassUISprite {
    uint screenDimension;  // Packed: high 16 bits width, low 16 bits height.
};
