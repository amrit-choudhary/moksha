/**
 * Read-only files packed inside the app package, for platforms whose resources aren't plain files (Android: the
 * APK's assets/ folder, read through the AAssetManager the driver puts in Utils::AppPaths). Only Vfs calls these;
 * defined only on those platforms (assets_android.cpp).
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ME::Platform {

// Size in bytes of the packed file relPath. False if assetManager is null or the file doesn't exist.
bool GetAssetSize(void* assetManager, const char* relPath, size_t& outSize);

// Reads the packed file relPath into a caller-owned buffer. False if it can't be opened, is larger than capacity or
// can't be fully read.
bool ReadAsset(void* assetManager, const char* relPath, uint8_t* buffer, size_t capacity, size_t& outSize);

}  // namespace ME::Platform
