#ifdef ME_MAC

#include <mach-o/dyld.h>

#include <climits>
#include <cstdint>
#include <cstdlib>

#include "exe_path.h"

std::string ME::Platform::GetExecutablePath(const char* argv0) {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        return argv0 != nullptr ? std::string{argv0} : std::string{};
    }

    // The loader's path may be relative or a symlink; the real file is where its resources sit.
    char resolved[PATH_MAX];
    if (realpath(path, resolved) == nullptr) {
        return std::string{path};
    }
    return std::string{resolved};
}

#endif  // ME_MAC
