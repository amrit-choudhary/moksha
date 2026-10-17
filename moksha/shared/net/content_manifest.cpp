#include "content_manifest.h"

#include <cmath>
#include <cstring>

#include "moksha/logging/logging.h"
#include "moksha/shared/third_party/json/cJSON.h"

namespace {

// Reads the "files" array into entries. Logs and returns false on the first invalid entry.
bool ReadEntries(const cJSON* root, ME::Net::ManifestEntry* entries, uint32_t maxEntries, uint32_t& outCount) {
    const cJSON* files = cJSON_GetObjectItemCaseSensitive(root, "files");
    if (!cJSON_IsArray(files)) {
        ME::LogError("Manifest: missing \"files\" array");
        return false;
    }

    const cJSON* file = nullptr;
    cJSON_ArrayForEach(file, files) {
        if (outCount >= maxEntries) {
            ME::LogError("Manifest: more than ", maxEntries, " files");
            return false;
        }

        const cJSON* name = cJSON_GetObjectItemCaseSensitive(file, "name");
        const cJSON* version = cJSON_GetObjectItemCaseSensitive(file, "version");
        if (!cJSON_IsString(name) || !cJSON_IsNumber(version)) {
            ME::LogError("Manifest: entry ", outCount, " needs a string \"name\" and a number \"version\"");
            return false;
        }

        if (!ME::Net::ContentManifest::IsSafePath(name->valuestring)) {
            ME::LogError("Manifest: unsafe file name \"", name->valuestring, "\"");
            return false;
        }

        // JSON numbers are doubles; accept only whole numbers that fit in uint32_t (NaN fails the range check).
        const double value = version->valuedouble;
        if (!(value >= 0.0 && value <= 4294967295.0) || value != std::floor(value)) {
            ME::LogError("Manifest: \"", name->valuestring, "\" version must be a whole number >= 0");
            return false;
        }

        for (uint32_t i = 0; i < outCount; ++i) {
            if (entries[i].name == name->valuestring) {
                ME::LogError("Manifest: \"", name->valuestring, "\" listed twice");
                return false;
            }
        }

        entries[outCount].name = name->valuestring;
        entries[outCount].version = static_cast<uint32_t>(value);
        ++outCount;
    }

    return true;
}

}  // namespace

bool ME::Net::ContentManifest::Parse(const char* text, size_t size) {
    Clear();

    cJSON* root = cJSON_ParseWithLength(text, size);
    if (root == nullptr) {
        LogError("Manifest: invalid JSON");
        return false;
    }

    const bool ok = ReadEntries(root, entries, MAX_ENTRIES, count);
    cJSON_Delete(root);

    if (!ok) {
        Clear();
    }
    return ok;
}

void ME::Net::ContentManifest::Clear() {
    for (uint32_t i = 0; i < count; ++i) {
        entries[i].name.clear();
    }
    count = 0;
}

bool ME::Net::ContentManifest::Serialize(std::string& out) const {
    cJSON* root = cJSON_CreateObject();
    cJSON* files = cJSON_AddArrayToObject(root, "files");
    if (root == nullptr || files == nullptr) {
        cJSON_Delete(root);
        return false;
    }

    for (uint32_t i = 0; i < count; ++i) {
        cJSON* file = cJSON_CreateObject();
        cJSON_AddStringToObject(file, "name", entries[i].name.c_str());
        cJSON_AddNumberToObject(file, "version", static_cast<double>(entries[i].version));
        cJSON_AddItemToArray(files, file);
    }

    // cJSON_Print allocates the text; it must be released with cJSON_free.
    char* text = cJSON_Print(root);
    cJSON_Delete(root);
    if (text == nullptr) {
        return false;
    }
    out = text;
    cJSON_free(text);
    return true;
}

bool ME::Net::ContentManifest::Set(const char* name, uint32_t version) {
    for (uint32_t i = 0; i < count; ++i) {
        if (entries[i].name == name) {
            entries[i].version = version;
            return true;
        }
    }

    if (count >= MAX_ENTRIES) {
        return false;
    }
    entries[count].name = name;
    entries[count].version = version;
    ++count;
    return true;
}

bool ME::Net::ContentManifest::Remove(const char* name) {
    for (uint32_t i = 0; i < count; ++i) {
        if (entries[i].name == name) {
            // Shift the following entries down one slot to close the gap.
            for (uint32_t j = i; j + 1 < count; ++j) {
                entries[j] = entries[j + 1];
            }
            --count;
            entries[count].name.clear();
            return true;
        }
    }
    return false;
}

const ME::Net::ManifestEntry* ME::Net::ContentManifest::Find(const char* name) const {
    for (uint32_t i = 0; i < count; ++i) {
        if (entries[i].name == name) {
            return &entries[i];
        }
    }
    return nullptr;
}

uint32_t ME::Net::ContentManifest::GetCount() const {
    return count;
}

const ME::Net::ManifestEntry& ME::Net::ContentManifest::GetEntry(uint32_t index) const {
    return entries[index];
}

bool ME::Net::ContentManifest::IsSafePath(const char* path) {
    const size_t length = std::strlen(path);
    if (length == 0 || length > MAX_PATH_LENGTH || path[0] == '/') {
        return false;
    }

    // Walk the path one '/'-separated segment at a time.
    size_t segmentStart = 0;
    for (size_t i = 0; i <= length; ++i) {
        const char c = path[i];
        if (c == '\\' || c == ':') {
            return false;
        }

        if (c == '/' || c == '\0') {
            const size_t segmentLength = i - segmentStart;
            const char* segment = path + segmentStart;
            const bool isDot = segmentLength == 1 && segment[0] == '.';
            const bool isDotDot = segmentLength == 2 && segment[0] == '.' && segment[1] == '.';
            if (segmentLength == 0 || isDot || isDotDot) {
                return false;
            }
            segmentStart = i + 1;
        }
    }
    return true;
}
