/**
 * Per-OS queries needed by every program, including servers and tools. OS calls for shared code live only in
 * this folder, one file per OS (exe_path_win.cpp, exe_path_posix.cpp).
 */
#pragma once

#include <string>

namespace ME::Platform {

// Full path of the running executable. argv0 is used where the OS has no direct query.
std::string GetExecutablePath(const char* argv0);

}  // namespace ME::Platform
