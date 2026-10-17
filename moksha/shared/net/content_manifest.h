/**
 * List of files a content server offers, parsed from manifest.json. Used by both server and client.
 * Format: { "files": [ { "name": "config/economy.json", "version": 3 }, ... ] }. Unknown keys are ignored.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ME::Net {

class ManifestEntry {
   public:
    std::string name;  // Path relative to the dlc/ root, '/' separators.
    uint32_t version = 0;
};

class ContentManifest {
   public:
    static constexpr uint32_t MAX_ENTRIES = 256;
    static constexpr size_t MAX_PATH_LENGTH = 255;

    /**
     * Replaces the current entries from JSON text (no terminating '\0' needed).
     * Returns false, leaving the manifest empty, if the JSON or any entry is invalid.
     */
    bool Parse(const char* text, size_t size);

    void Clear();

    /**
     * Writes the entries as JSON in the same format Parse reads. Returns false if JSON creation failed.
     */
    bool Serialize(std::string& out) const;

    /**
     * Adds name with this version, or updates its version if already listed. name must be a safe path.
     * Returns false if the manifest is full.
     */
    bool Set(const char* name, uint32_t version);

    /**
     * Removes the entry with this name, keeping the order of the others. Returns false if it isn't listed.
     */
    bool Remove(const char* name);

    /**
     * Entry with exactly this name, or nullptr.
     */
    const ManifestEntry* Find(const char* name) const;

    uint32_t GetCount() const;

    /**
     * index must be below GetCount().
     */
    const ManifestEntry& GetEntry(uint32_t index) const;

    /**
     * True for a relative path that stays inside its root: not empty, no leading '/', no '\' or ':',
     * no empty, "." or ".." segments, at most MAX_PATH_LENGTH characters.
     */
    static bool IsSafePath(const char* path);

   private:
    ManifestEntry entries[MAX_ENTRIES];
    uint32_t count = 0;
};

}  // namespace ME::Net
