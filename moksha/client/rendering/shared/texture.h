/**
 * Texture class for handling textures.
 * This is platform agnostic. This will contain common code for all platforms.
 */

#pragma once

#include <cstdint>

namespace ME {

// Constants.
constexpr uint8_t MIP_LEVELS_DEFAULT = 4;

enum class TextureAtlasPaddingType : uint8_t {
    None = 0,
    No_Edge = 1,  // No edge padding, just padding between tiles
    Edge = 2      // Edge padding, with padding between tiles
};

/**
 * Data related to texture atlases.
 * This is used to hold information about the texture atlas, such as the size, number of textures, etc.
 * fields are uint32_t because this goes to shaders as a constant buffer, where 4-byte alignment is preferred.
 */
class TextureAtlasProperties {
   public:
    uint32_t tileSizeX;      // Width of each tile in the atlas
    uint32_t tileSizeY;      // Size of each tile in the atlas
    uint32_t padding;        // Padding between tiles in the atlas
    uint32_t numTextures;    // Number of textures in the atlas
    uint32_t numTilesX;      // Number of tiles in the X direction
    uint32_t numTilesY;      // Number of tiles in the Y direction
    uint32_t width;          // Width of the atlas texture
    uint32_t height;         // Height of the atlas texture
    uint32_t paddingType;    // Type of padding used in the atlas (see TextureAtlasPaddingType)
    uint32_t gpuPadding[3];  // Padding to align the structure to 16 bytes
};

class Texture {
   public:
    Texture();
    Texture(const char* path);
    virtual ~Texture();

    // Load the texture from a file, using the libpng library.
    void Load(const char* path);

    uint16_t GetWidth() const {
        return width;
    }
    uint16_t GetHeight() const {
        return height;
    }
    uint8_t GetChannels() const {
        return channels;
    }

    bool HasAlpha() const {
        return hasAlpha;
    }

    const uint32_t GetSize() const {
        return width * height * channels;
    }

    const uint8_t* GetData() const {
        return data;
    }

   protected:
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t channels = 0;
    uint32_t bytesPerPixel = 0;
    bool hasAlpha = false;
    uint8_t* data = nullptr;
};

}  // namespace ME
