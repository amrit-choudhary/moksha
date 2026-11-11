// Constant layouts shared by the GLSL shaders; GLSL port of structs.hlsl: keep them in sync.
// They follow std140 and match the C++ structs (shader_constants.h, sprite_renderer.h) byte for byte.

struct CBPerPass {
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

struct CBPerSprite {
    mat4 modelMatrix;
    vec4 color;
    uint atlasIndex;
    uint flags;
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
};

// Per-instance data for instanced sprites; 96 bytes, matching SpriteRendererInstanceData.
struct SpriteInstanceData {
    mat4 modelMatrix;
    vec4 color;
    uint atlasIndex;
    uint flags;
    uvec2 padding;
};

// Per-instance data for UI sprites and text; the same 96-byte layout as SpriteInstanceData.
struct UISpriteInstanceData {
    mat4 modelMatrix;
    vec4 color;
    uint atlasIndex;
    uint flags;
    uvec2 padding;
};

struct TextInstanceData {
    mat4 modelMatrix;
    vec4 color;
    uint atlasIndex;
    uint flags;
    uvec2 padding;
};
