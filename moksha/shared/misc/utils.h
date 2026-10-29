/**
 * Global utility functions.
 */
#pragma once

#include <cstdint>
#include <string>

namespace ME::Utils {

/**
 * Converts a c style string to uint32_t using hashing function.
 */
uint32_t HashString2uint32(const char* str);

/**
 * The folders the file roots map to (see Vfs), each with a trailing slash, and the executable's folder without one.
 * Each platform driver fills it in; the OS decides where writable data may live.
 */
class AppPaths {
   public:
    std::string executableDir;
    std::string resources;
    std::string dlc;
    std::string userData;
};

/**
 * Desktop and server layout: resources/, dlc/ and user_data/ next to the executable at executablePath.
 */
AppPaths MakeExecutableRelativePaths(const std::string& executablePath);

/**
 * Sets the folders the getters below return. Call once at startup, before any file access.
 */
void SetPaths(const AppPaths& paths);

/**
 * Read-only folder of the shipped, cooked resources, with a trailing slash.
 */
std::string GetResourcesPath();

/**
 * Folder for downloadable content fetched from a content server, with a trailing slash.
 */
std::string GetDlcPath();

/**
 * Writable folder for saves and user settings, with a trailing slash. Desktop: user_data/ next to resources/.
 */
std::string GetUserDataPath();

/**
 * Directory containing the executable, without a trailing slash.
 */
std::string GetExecutableDirPath();

inline uint32_t Pack16To32(uint16_t high, uint16_t low) {
    return (static_cast<uint32_t>(high) << 16) | static_cast<uint32_t>(low);
}

inline void Unpack32To16(uint32_t packed, uint16_t& out_high, uint16_t& out_low) {
    out_high = static_cast<uint16_t>((packed >> 16) & 0xFFFF);
    out_low = static_cast<uint16_t>(packed & 0xFFFF);
}

}  // namespace ME::Utils
