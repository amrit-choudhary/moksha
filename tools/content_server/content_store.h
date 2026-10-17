/**
 * Everything the content server serves, loaded into memory once at startup from dlc/:
 * the manifest (parsed + raw text sent to clients) and the bytes of every listed file.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/shared/net/content_manifest.h"

namespace ME {

class StoredFile {
   public:
    uint8_t* data = nullptr;  // Owned by ContentStore; nullptr if the file failed to load.
    size_t size = 0;
};

class ContentStore {
   public:
    ContentStore() = default;
    ~ContentStore();

    // Owns heap memory for file bytes; copying would free it twice.
    ContentStore(const ContentStore&) = delete;
    ContentStore& operator=(const ContentStore&) = delete;

    /**
     * Reads dlc/manifest.json and every file it lists. Returns false if the manifest is missing or invalid;
     * individual files that are missing or too large are logged and skipped.
     */
    bool Load();

    const Net::ContentManifest& GetManifest() const;

    /**
     * Raw manifest.json text, sent unchanged to clients as the MANIFEST message.
     */
    const std::string& GetManifestText() const;

    /**
     * Loaded bytes for a manifest file name, or nullptr if it isn't listed or failed to load.
     */
    const StoredFile* FindFile(const char* name) const;

   private:
    bool LoadFile(uint32_t index);
    void Clear();

    Net::ContentManifest manifest;
    std::string manifestText;

    // Same order as the manifest entries: files[i] holds the bytes of manifest.GetEntry(i).
    StoredFile files[Net::ContentManifest::MAX_ENTRIES];
};

}  // namespace ME
