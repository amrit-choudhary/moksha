#include "tile.h"

ME::TileRenderData ME::TileRenderData::FromGlobalTileID(uint32_t globalTileID) {
    TileRenderData tileData;
    tileData.atlasIndex = globalTileID & ~(FLIP_X_FLAG | FLIP_Y_FLAG | FLIP_DIAGONAL_FLAG | ROTATED_HEXAGONAL_FLAG);
    // Our tile atlas is zero-indexed, while TMX global tile IDs start from 1.
    if (tileData.atlasIndex > 0) {
        tileData.atlasIndex -= 1;
    }
    tileData.flipX = (globalTileID & FLIP_X_FLAG) ? 1 : 0;
    tileData.flipY = (globalTileID & FLIP_Y_FLAG) ? 1 : 0;
    tileData.flipDiagonal = (globalTileID & FLIP_DIAGONAL_FLAG) ? 1 : 0;
    tileData.rotatedHexagonal = (globalTileID & ROTATED_HEXAGONAL_FLAG) ? 1 : 0;
    return tileData;
}
