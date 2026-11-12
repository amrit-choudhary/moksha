#ifdef ME_ANDROID

#include <android/asset_manager.h>

#include "assets.h"

namespace {

AAsset* OpenAsset(void* assetManager, const char* relPath) {
    if (assetManager == nullptr || relPath == nullptr) {
        return nullptr;
    }
    return AAssetManager_open(static_cast<AAssetManager*>(assetManager), relPath, AASSET_MODE_STREAMING);
}

}  // namespace

bool ME::Platform::GetAssetSize(void* assetManager, const char* relPath, size_t& outSize) {
    AAsset* asset = OpenAsset(assetManager, relPath);
    if (asset == nullptr) {
        return false;
    }
    const off64_t length = AAsset_getLength64(asset);
    AAsset_close(asset);
    if (length < 0) {
        return false;
    }
    outSize = static_cast<size_t>(length);
    return true;
}

bool ME::Platform::ReadAsset(void* assetManager, const char* relPath, uint8_t* buffer, size_t capacity,
                             size_t& outSize) {
    AAsset* asset = OpenAsset(assetManager, relPath);
    if (asset == nullptr) {
        return false;
    }
    const off64_t length = AAsset_getLength64(asset);
    if (length < 0 || static_cast<uint64_t>(length) > capacity) {
        AAsset_close(asset);
        return false;
    }

    // A compressed asset is inflated as it is read, so one call may return fewer bytes than asked for.
    const size_t size = static_cast<size_t>(length);
    size_t total = 0;
    while (total < size) {
        const int read = AAsset_read(asset, buffer + total, size - total);
        if (read <= 0) {
            break;
        }
        total += static_cast<size_t>(read);
    }
    AAsset_close(asset);
    if (total != size) {
        return false;
    }
    outSize = size;
    return true;
}

#endif  // ME_ANDROID
