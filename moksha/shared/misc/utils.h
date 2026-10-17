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
 * Sets important game directory paths.
 */
void SetPaths(char* arg1, char* arg2);

std::string GetResourcesPath();

/**
 * Folder for downloadable content fetched from a content server, next to resources/. Has a trailing slash.
 */
std::string GetDlcPath();

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
