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
