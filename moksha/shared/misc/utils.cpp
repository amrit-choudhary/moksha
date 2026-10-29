#include "utils.h"

#include <string>

namespace {
// The folders set at startup.
static ME::Utils::AppPaths appPaths;
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

ME::Utils::AppPaths ME::Utils::MakeExecutableRelativePaths(const std::string& executablePath) {
    AppPaths paths;

    // Cut at the last separator to drop the executable name, whatever its length.
    // Windows paths use '\', Mac/Linux use '/'. No separator means the current directory.
    const size_t lastSeparator = executablePath.find_last_of("/\\");
    if (lastSeparator == std::string::npos) {
        paths.executableDir = ".";
    } else {
        paths.executableDir = executablePath.substr(0, lastSeparator);
    }

    paths.resources = paths.executableDir + "/resources/";
    paths.dlc = paths.executableDir + "/dlc/";
    paths.userData = paths.executableDir + "/user_data/";
    return paths;
}

void ME::Utils::SetPaths(const AppPaths& paths) {
    appPaths = paths;
}

std::string ME::Utils::GetResourcesPath() {
    return appPaths.resources;
}

std::string ME::Utils::GetDlcPath() {
    return appPaths.dlc;
}

std::string ME::Utils::GetUserDataPath() {
    return appPaths.userData;
}

std::string ME::Utils::GetExecutableDirPath() {
    return appPaths.executableDir;
}
