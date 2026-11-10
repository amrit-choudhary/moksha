// Common helper functions for GLSL shaders: the counterpart of helpers.hlsl and helpers.wgsl. Include structs.glsl first.

// UV inside one tile of an atlas. atlasIndex counts tiles from the top left; uv is the quad's 0..1 UV.
// flags: bit 0 flips horizontally, bit 1 vertically.
vec2 GetAtlasUV(vec2 uv, uint atlasIndex, TextureAtlasProperties atlasProps, uint flags) {
    float tileW = float(atlasProps.tileSizeX);
    float tileH = float(atlasProps.tileSizeY);
    float pad = float(atlasProps.padding);  // padding in pixels

    // cell stride in pixels
    float strideX = tileW + pad;
    float strideY = tileH + pad;

    // tile cell coordinates
    uint col = atlasIndex % atlasProps.numTilesX;
    uint row = atlasIndex / atlasProps.numTilesX;

    // origin of the tile cell in atlas pixels
    vec2 origin = vec2(float(col) * strideX, float(row) * strideY);

    // normalized uv -> local pixel coordinates inside the tile, flipped in pixel space
    vec2 localPx = uv * vec2(tileW, tileH);
    if ((flags & 1u) != 0u) {
        localPx.x = tileW - localPx.x;
    }
    if ((flags & 2u) != 0u) {
        localPx.y = tileH - localPx.y;
    }

    return (origin + localPx) / vec2(float(atlasProps.width), float(atlasProps.height));
}
