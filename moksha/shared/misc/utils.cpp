#include "utils.h"

#include <string>

namespace {
// Full path of the executable.
static std::string executablePath;

// Directory containing the executable.
static std::string executableDirPath;

// Path of resource folder.
static std::string resourceDirPath;

// Path of dlc folder (downloadable content fetched from a content server).
static std::string dlcDirPath;
}  // namespace

using namespace ME;

uint32_t ME::Utils::HashString2uint32(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void ME::Utils::SetPaths(char* arg1, char* arg2) {
    executablePath = arg1;

    executableDirPath = std::string{executablePath};

    // Cut at the last separator to drop the executable name, whatever its length.
    // Windows paths use '\', Mac/Linux use '/'. No separator means the current directory.
    const size_t lastSeparator = executableDirPath.find_last_of("/\\");
    if (lastSeparator == std::string::npos) {
        executableDirPath = ".";
    } else {
        executableDirPath.resize(lastSeparator);
    }

    resourceDirPath = std::string{executableDirPath};
    resourceDirPath += "/resources/";

    dlcDirPath = std::string{executableDirPath};
    dlcDirPath += "/dlc/";
}

std::string ME::Utils::GetResourcesPath() {
    return resourceDirPath;
}

std::string ME::Utils::GetDlcPath() {
    return dlcDirPath;
}

std::string ME::Utils::GetExecutableDirPath() {
    return executableDirPath;
}
