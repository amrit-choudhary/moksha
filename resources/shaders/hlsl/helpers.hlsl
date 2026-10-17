// contains some common helper functions and macros for HLSL shaders.

// Function to calculate UV coordinates for a texture atlas.
// Atlas index is the index of the texture in the atlas, starting from top left.
// uv will simply be quad UVs from (0,0) to (1,1).

float2 GetAtlasUV(float2 uv, uint atlasIndex, TextureAtlasProperties atlasProps, uint flags)
{
    // pixel sizes
    float tileW = (float)atlasProps.tileSizeX;
    float tileH = (float)atlasProps.tileSizeY;
    float pad   = (float)atlasProps.padding; // padding in pixels

    // cell stride in pixels
    float strideX = tileW + pad;
    float strideY = tileH + pad;

    // tile cell coordinates
    uint col = atlasIndex % atlasProps.numTilesX;
    uint row = atlasIndex / atlasProps.numTilesX;

    // origin of the tile cell in atlas pixels
    float originX = col * strideX;
    float originY = row * strideY;

    // convert normalized uv -> local pixel coordinates inside tile content
    float2 localPx = uv * float2(tileW, tileH);

    // apply horizontal / vertical flip in pixel space
    bool flipH = (flags & 0x1u) != 0u;
    bool flipV = (flags & 0x2u) != 0u;
    if (flipH) localPx.x = tileW - localPx.x;
    if (flipV) localPx.y = tileH - localPx.y;

    // map to atlas pixel coords and convert to normalized UV
    float2 atlasPx = float2(originX, originY) + localPx;
    float2 atlasUV = atlasPx / float2((float)atlasProps.width, (float)atlasProps.height);

    return atlasUV;
}
