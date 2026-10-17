#include "content_store.h"

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"
#include "moksha/shared/net/content_protocol.h"
#include "moksha/shared/net/message_framing.h"

namespace {
constexpr const char* MANIFEST_FILE = "manifest.json";

// Bytes in a FILE message besides the file itself: header + u16 path length + the path.
size_t FileMessageOverhead(const std::string& name) {
    return ME::Net::FRAME_HEADER_SIZE + sizeof(uint16_t) + name.size();
}
}  // namespace

ME::ContentStore::~ContentStore() {
    Clear();
}

bool ME::ContentStore::Load() {
    Clear();

    if (!Vfs::ReadText(FileRoot::Dlc, MANIFEST_FILE, manifestText)) {
        LogError("Can't open ", Vfs::GetRootPath(FileRoot::Dlc), MANIFEST_FILE);
        return false;
    }

    if (Net::FRAME_HEADER_SIZE + manifestText.size() > Net::ContentProtocol::MAX_MESSAGE_SIZE) {
        LogError(MANIFEST_FILE, " is too large to send (", manifestText.size(), " bytes)");
        Clear();
        return false;
    }

    if (!manifest.Parse(manifestText.data(), manifestText.size())) {
        Clear();
        return false;
    }

    uint32_t loadedCount = 0;
    size_t totalBytes = 0;
    for (uint32_t i = 0; i < manifest.GetCount(); ++i) {
        if (LoadFile(i)) {
            ++loadedCount;
            totalBytes += files[i].size;
        }
    }

    if (loadedCount == manifest.GetCount()) {
        LogSuccess("Serving ", loadedCount, " files (", totalBytes, " bytes)");
    } else {
        LogWarning("Serving ", loadedCount, " of ", manifest.GetCount(), " files (", totalBytes,
                   " bytes); clients can't finish syncing until the missing files are fixed");
    }
    return true;
}

bool ME::ContentStore::LoadFile(uint32_t index) {
    const Net::ManifestEntry& entry = manifest.GetEntry(index);
    const char* name = entry.name.c_str();

    size_t size = 0;
    if (!Vfs::GetFileSize(FileRoot::Dlc, name, size)) {
        LogWarning("Missing: ", name);
        return false;
    }

    // The whole FILE message (header + path + bytes) must fit in one message.
    const size_t maxSize = Net::ContentProtocol::MAX_MESSAGE_SIZE - FileMessageOverhead(entry.name);
    if (size > maxSize) {
        LogWarning("Too large: ", name, " (", size, " bytes, max ", maxSize, ")");
        return false;
    }

    uint8_t* data = new uint8_t[size];
    size_t readSize = 0;
    if (!Vfs::ReadBytes(FileRoot::Dlc, name, data, size, readSize)) {
        LogWarning("Read failed: ", name);
        delete[] data;
        return false;
    }

    files[index].data = data;
    files[index].size = readSize;
    LogInfo("Loaded ", name, " v", entry.version, " (", readSize, " bytes)");
    return true;
}

const ME::Net::ContentManifest& ME::ContentStore::GetManifest() const {
    return manifest;
}

const std::string& ME::ContentStore::GetManifestText() const {
    return manifestText;
}

const ME::StoredFile* ME::ContentStore::FindFile(const char* name) const {
    for (uint32_t i = 0; i < manifest.GetCount(); ++i) {
        if (manifest.GetEntry(i).name == name) {
            return files[i].data != nullptr ? &files[i] : nullptr;
        }
    }
    return nullptr;
}

void ME::ContentStore::Clear() {
    for (StoredFile& file : files) {
        delete[] file.data;
        file.data = nullptr;
        file.size = 0;
    }
    manifestText.clear();
    manifest.Clear();
}
