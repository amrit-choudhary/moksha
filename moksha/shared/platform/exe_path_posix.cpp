#if !defined(ME_WIN) && !defined(ME_MAC)

#include "exe_path.h"

std::string ME::Platform::GetExecutablePath(const char* argv0) {
    return argv0 != nullptr ? std::string{argv0} : std::string{};
}

#endif  // !ME_WIN && !ME_MAC
