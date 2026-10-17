#ifdef ME_WIN

#include "exe_path.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

std::string ME::Platform::GetExecutablePath(const char* argv0) {
    char path[MAX_PATH];
    const DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return argv0 != nullptr ? std::string{argv0} : std::string{};
    }
    return std::string{path, length};
}

#endif  // ME_WIN
