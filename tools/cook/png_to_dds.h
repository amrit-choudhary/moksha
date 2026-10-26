/**
 * PNG to DDS without texconv: decodes a PNG to RGBA8 and writes it as an uncompressed DDS, the layout
 * `texconv -f R8G8B8A8_UNORM -m 1 -ft DDS` makes, so the runtime's DDSParser loads it the same on every platform.
 */
#pragma once

#include <filesystem>
#include <string>

namespace ME::Cook {

// Converts the PNG at src into a DDS at dst (replacing it). False with outError set on failure.
bool ConvertPngToDds(const std::filesystem::path& src, const std::filesystem::path& dst, std::string& outError);

}  // namespace ME::Cook
