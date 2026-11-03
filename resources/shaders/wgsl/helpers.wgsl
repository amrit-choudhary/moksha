// Common helper functions for WGSL shaders: the counterpart of helpers.hlsl and helpers.metal. Include structs.wgsl first.

// Function to calculate UV coordinates for a texture atlas.
// Atlas index is the index of the texture in the atlas, starting from top left.
// uv will simply be quad UVs from (0,0) to (1,1).
fn GetAtlasUV(uv: vec2f, atlasIndex: u32, atlasProps: TextureAtlasProperties, flags: u32) -> vec2f {
    // pixel sizes
    let tileW = f32(atlasProps.tileSizeX);
    let tileH = f32(atlasProps.tileSizeY);
    let pad = f32(atlasProps.padding);  // padding in pixels

    // cell stride in pixels
    let strideX = tileW + pad;
    let strideY = tileH + pad;

    // tile cell coordinates
    let col = atlasIndex % atlasProps.numTilesX;
    let row = atlasIndex / atlasProps.numTilesX;

    // origin of the tile cell in atlas pixels
    let originX = f32(col) * strideX;
    let originY = f32(row) * strideY;

    // convert normalized uv -> local pixel coordinates inside tile content
    var localPx = uv * vec2f(tileW, tileH);

    // apply horizontal / vertical flip in pixel space
    if ((flags & 1u) != 0u) {
        localPx.x = tileW - localPx.x;
    }
    if ((flags & 2u) != 0u) {
        localPx.y = tileH - localPx.y;
    }

    // map to atlas pixel coords and convert to normalized UV
    let atlasPx = vec2f(originX, originY) + localPx;
    return atlasPx / vec2f(f32(atlasProps.width), f32(atlasProps.height));
}

// Maps a UI position in pixels from the screen's centre to clip space. Screen dimensions are packed in a single u32.
fn GetUIPosition(position: vec4f, screenDimension: u32) -> vec4f {
    let screenWidth = f32((screenDimension >> 16u) & 0xFFFFu);
    let screenHeight = f32(screenDimension & 0xFFFFu);
    return vec4f((position.x / screenWidth) * 2.0, (position.y / screenHeight) * 2.0, position.z, position.w);
}
