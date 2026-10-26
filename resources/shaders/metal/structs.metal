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

// The C++ instance data is 88 bytes with 4-byte alignment; packed columns keep that stride (a float4x4 member would
// pad it to 96). The matrix columns are column-major, from Mat4::GetDataForShader; GetModelMatrix rebuilds it.

// Per Instance Data for Sprite Renderer
struct SpriteInstanceData {
    packed_float4 modelColumn0;
    packed_float4 modelColumn1;
    packed_float4 modelColumn2;
    packed_float4 modelColumn3;
    packed_float4 color;
    uint atlasIndex;
    uint flags;
};

// Per Instance Data for UI Sprite Renderer
struct UISpriteInstanceData {
    packed_float4 modelColumn0;
    packed_float4 modelColumn1;
    packed_float4 modelColumn2;
    packed_float4 modelColumn3;
    packed_float4 color;
    uint atlasIndex;
    uint flags;
};

// Per Instance Data for Text Renderer
struct TextInstanceData {
    packed_float4 modelColumn0;
    packed_float4 modelColumn1;
    packed_float4 modelColumn2;
    packed_float4 modelColumn3;
    packed_float4 color;
    uint atlasIndex;
    uint flags;
};

// The model matrix of any of the instance data structs above.
template <typename InstanceData>
float4x4 GetModelMatrix(InstanceData instance) {
    return float4x4(float4(instance.modelColumn0), float4(instance.modelColumn1), float4(instance.modelColumn2),
                    float4(instance.modelColumn3));
}

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
