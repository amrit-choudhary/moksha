/**
 * PNG to DDS: decodes a PNG to RGBA8 and writes it as an uncompressed DDS with one mip, so the runtime's DDSParser
 * loads it the same on every platform.
 */
#pragma once

#include <filesystem>
#include <string>

namespace ME::Cook {

// Converts the PNG at src into a DDS at dst (replacing it). False with outError set on failure.
bool ConvertPngToDds(const std::filesystem::path& src, const std::filesystem::path& dst, std::string& outError);

}  // namespace ME::Cook
