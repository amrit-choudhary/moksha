// The WGSL version of game side structs used for rendering: the counterpart of structs.hlsl and structs.metal. Layouts
// match the C++ classes (rendering/shared/texture.h, sprite_renderer.h, text_renderer.h, shader_constants.h), but declare
// only the fields shaders read: a buffer may be bigger than its struct, and uniform arrays have a 16-byte stride, so the
// C++ padding arrays can't be mirrored as arrays.

// Per Pass Constant Buffer. The lights follow the two matrices in the buffer.
struct CBPerPass {
    viewMatrix: mat4x4f,
    projectionMatrix: mat4x4f,
}

// Per Pass Constant Buffer for UI Text and UI Sprite
struct CBPerPassUI {
    screenDimension: u32,  // Packed: high 16 bits width, low 16 bits height.
}

// Texture Atlas Properties
struct TextureAtlasProperties {
    tileSizeX: u32,
    tileSizeY: u32,
    padding: u32,
    numTextures: u32,
    numTilesX: u32,
    numTilesY: u32,
    width: u32,
    height: u32,
    paddingType: u32,
}

// Per Sprite Constant Buffer
struct CBPerSprite {
    modelMatrix: mat4x4f,
    color: vec4f,
    atlasIndex: u32,
    flags: u32,
}

// Per Instance Data for the Sprite, UI Sprite and Text Renderers: 96 bytes.
struct InstanceData {
    modelMatrix: mat4x4f,
    color: vec4f,
    atlasIndex: u32,
    flags: u32,
    padding: vec2u,
}

struct VertexIn {
    @location(0) position: vec2f,
    @location(1) uv: vec2f,
}

struct VertexOut {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
    @location(1) color: vec4f,
}
