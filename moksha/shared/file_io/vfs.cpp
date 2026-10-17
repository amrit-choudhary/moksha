#include "vfs.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include "moksha/shared/misc/utils.h"

std::string ME::Vfs::GetRootPath(FileRoot root) {
    switch (root) {
        case FileRoot::Resources:
            return Utils::GetResourcesPath();
        case FileRoot::Dlc:
            return Utils::GetDlcPath();
    }
    return Utils::GetResourcesPath();
}

bool ME::Vfs::ReadText(FileRoot root, const char* relPath, std::string& out) {
    // Text mode: on Windows "\r\n" line endings are read as "\n", which the parsers expect.
    std::ifstream file(GetRootPath(root) + relPath);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

bool ME::Vfs::GetFileSize(FileRoot root, const char* relPath, size_t& outSize) {
    // ios::ate opens with the read position at the end, so tellg() is the file size.
    std::ifstream file(GetRootPath(root) + relPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    const std::streamoff size = file.tellg();
    if (size < 0) {
        return false;
    }
    outSize = static_cast<size_t>(size);
    return true;
}

bool ME::Vfs::ReadBytes(FileRoot root, const char* relPath, uint8_t* buffer, size_t capacity, size_t& outSize) {
    std::ifstream file(GetRootPath(root) + relPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    const std::streamoff size = file.tellg();
    if (size < 0 || static_cast<uint64_t>(size) > capacity) {
        return false;
    }

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer), size);
    if (file.gcount() != size) {
        return false;
    }

    outSize = static_cast<size_t>(size);
    return true;
}

bool ME::Vfs::WriteBytes(FileRoot root, const char* relPath, const uint8_t* data, size_t size) {
    const std::filesystem::path path(GetRootPath(root) + relPath);

    // Exceptions are disabled, so std::filesystem calls must use the std::error_code overload;
    // the throwing overload would terminate the program on failure.
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    // close() writes out bytes still buffered in memory; a failure there (e.g. disk full) shows up in fail().
    file.close();
    return !file.fail();
}

bool ME::Vfs::RemoveFile(FileRoot root, const char* relPath) {
    std::error_code error;
    // remove() returns false without an error when the file doesn't exist; that still counts as success.
    std::filesystem::remove(std::filesystem::path(GetRootPath(root) + relPath), error);
    return !error;
}

void ME::Vfs::RemoveEmptyFolders(FileRoot root, const char* relPath) {
    // Walk the relative path's folders from deepest to shallowest, e.g. "a/b/c.json" -> "a/b", then "a".
    std::filesystem::path folder = std::filesystem::path(relPath).parent_path();
    const std::string rootPath = GetRootPath(root);

    std::error_code error;
    while (!folder.empty()) {
        const std::filesystem::path fullPath = std::filesystem::path(rootPath) / folder;
        if (!std::filesystem::is_empty(fullPath, error) || error) {
            return;
        }
        std::filesystem::remove(fullPath, error);
        if (error) {
            return;
        }
        folder = folder.parent_path();
    }
}
