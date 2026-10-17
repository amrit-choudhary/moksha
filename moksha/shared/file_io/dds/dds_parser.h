#pragma once

/**
 * DDS file parser.
 * Supports loading DDS files with mipmaps and various formats.
 */

#include <cstdint>

namespace ME {

class DDSParser {
   public:
    /**
     * Function to load a DDS file and print basic info
     * Assumptions: Only supports uncompressed DX10 and R8G8B8A8 formats.
     */
    static bool LoadDDS(const char* filePath, uint8_t** outData, uint32_t* outWidth, uint32_t* outHeight,
                        uint32_t* bytesPerPixel, uint8_t* channels);
};

}  // namespace ME
