/**
 * DEPRECATED: Using platform specific image loading now.
 *
 * Loads PNG files and converts them to a common image format
 */

#pragma once

#include <vector>

namespace ME {

// Structure to hold loaded image data
struct PNGData {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels;  // Raw pixel data (usually RGBA)
    bool has_alpha = false;
    int channels = 0;  // Number of channels in the output buffer (e.g., 4 for RGBA)
};

bool LoadPNG(const char* fileName, PNGData& out_data);

}  // namespace ME
