#pragma once

/**
 * Tile representation in the game world.
 */

#include <cstdint>

namespace ME {

/**
 * Data structure for a tile in tilemap.
 */
class TileData {
   public:
    TileData() = default;
    ~TileData() = default;

    bool walkable = true;
};

/**
 * Data for rendering a tile in tilemap.
 * Follows the TMX format.
 * Ref: https://doc.mapeditor.org/en/stable/reference/tmx-map-format/#tmx-tilelayer-tile
 * Flip flags are logically bools but stored as uint8_t for sending it to GPU. Can be 0 or 1.
 */
class TileRenderData {
   public:
    TileRenderData() = default;
    ~TileRenderData() = default;

    bool visible = true;
    // Index in the texture atlas.
    uint32_t atlasIndex = 0;
    // Horizontal flip the sprite.
    uint8_t flipX = 0;
    // Vertical flip the sprite.
    uint8_t flipY = 0;
    // Diagonal flip the sprite.
    uint8_t flipDiagonal = 0;
    // Rotated hexagonal tile.
    uint8_t rotatedHexagonal = 0;

    static constexpr uint32_t FLIP_X_FLAG = 0x80000000;
    static constexpr uint32_t FLIP_Y_FLAG = 0x40000000;
    static constexpr uint32_t FLIP_DIAGONAL_FLAG = 0x20000000;
    static constexpr uint32_t ROTATED_HEXAGONAL_FLAG = 0x10000000;

    /** Create TileRenderData from a global tile ID. Following TMX format. */
    static TileRenderData FromGlobalTileID(uint32_t globalTileID);
};

class Tile {
   public:
    Tile() = default;
    ~Tile() = default;
};

}  // namespace ME
