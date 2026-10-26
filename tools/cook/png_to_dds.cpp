#include "png_to_dds.h"

#include <climits>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "stb_image.h"

namespace {

// DDS header fields (Microsoft's DDS_HEADER / DDS_PIXELFORMAT), written little-endian like every target.
constexpr uint32_t kDdsMagic = 0x20534444;                         // "DDS "
constexpr uint32_t kHeaderFlags = 0x1 | 0x2 | 0x4 | 0x8 | 0x1000;  // caps, height, width, pitch, pixel format
constexpr uint32_t kPixelFormatFlags = 0x1 | 0x40;                 // alpha pixels, RGB
constexpr uint32_t kCapsTexture = 0x1000;

#pragma pack(push, 1)
class DdsPixelFormat {
   public:
    uint32_t size = 32;
    uint32_t flags = kPixelFormatFlags;
    uint32_t fourCC = 0;
    uint32_t rgbBitCount = 32;
    uint32_t rBitMask = 0x000000FF;
    uint32_t gBitMask = 0x0000FF00;
    uint32_t bBitMask = 0x00FF0000;
    uint32_t aBitMask = 0xFF000000;
};

class DdsHeader {
   public:
    uint32_t size = 124;
    uint32_t flags = kHeaderFlags;
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitch = 0;
    uint32_t depth = 0;
    uint32_t mipMapCount = 1;
    uint32_t reserved1[11] = {};
    DdsPixelFormat format;
    uint32_t caps = kCapsTexture;
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;
};
#pragma pack(pop)

static_assert(sizeof(DdsPixelFormat) == 32, "DDS_PIXELFORMAT is 32 bytes");
static_assert(sizeof(DdsHeader) == 124, "DDS_HEADER is 124 bytes");

}  // namespace

bool ME::Cook::ConvertPngToDds(const std::filesystem::path& src, const std::filesystem::path& dst,
                               std::string& outError) {
    std::ifstream in(src, std::ios::binary);
    if (!in.is_open()) {
        outError = "can't read " + src.generic_string();
        return false;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    const std::string bytes = buffer.str();
    if (bytes.size() > static_cast<size_t>(INT_MAX)) {
        outError = "file too large";
        return false;
    }

    int width = 0;
    int height = 0;
    int fileChannels = 0;
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes.data()),
                                            static_cast<int>(bytes.size()), &width, &height, &fileChannels, 4);
    if (pixels == nullptr) {
        outError = std::string("PNG decode failed: ") + stbi_failure_reason();
        return false;
    }

    DdsHeader header;
    header.width = static_cast<uint32_t>(width);
    header.height = static_cast<uint32_t>(height);
    header.pitch = header.width * 4;

    std::ofstream out(dst, std::ios::binary | std::ios::trunc);
    if (out.is_open()) {
        out.write(reinterpret_cast<const char*>(&kDdsMagic), sizeof(kDdsMagic));
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(pixels), static_cast<std::streamsize>(header.pitch) * header.height);
    }
    const bool written = out.is_open() && out.good();
    stbi_image_free(pixels);
    if (!written) {
        outError = "can't write " + dst.generic_string();
        return false;
    }
    return true;
}
