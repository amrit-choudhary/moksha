#include "dds_parser.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "moksha/shared/misc/utils.h"

#pragma pack(push, 1)
struct DDS_PIXELFORMAT {
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t rgbBitCount;
    uint32_t rBitMask;
    uint32_t gBitMask;
    uint32_t bBitMask;
    uint32_t aBitMask;
};

struct DDS_HEADER {
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    DDS_PIXELFORMAT format;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
};

struct DDS_HEADER_DX10 {
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t miscFlags2;
};

#pragma pack(pop)

constexpr uint32_t MAKE_FOURCC(char a, char b, char c, char d) {
    return static_cast<uint32_t>(a) | (static_cast<uint32_t>(b) << 8) | (static_cast<uint32_t>(c) << 16) |
           (static_cast<uint32_t>(d) << 24);
}
constexpr uint32_t FOURCC_DDS = MAKE_FOURCC('D', 'D', 'S', ' ');
constexpr uint32_t FOURCC_DX10 = MAKE_FOURCC('D', 'X', '1', '0');

bool ME::DDSParser::LoadDDS(const char* fileName, uint8_t** outData, uint32_t* outWidth, uint32_t* outHeight,
                            uint32_t* bytesPerPixel, uint8_t* channels) {
    std::string fileNameString = ME::Utils::GetResourcesPath() + fileName;
    const char* filename = fileNameString.c_str();

    std::ifstream fs(filename, std::ios::binary);
    if (!fs) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }

    uint32_t magic;
    fs.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    if (magic != FOURCC_DDS) {
        std::cerr << "Not a valid DDS file: " << fileName << std::endl;
        return false;
    }

    DDS_HEADER header;
    fs.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.format.fourCC == FOURCC_DX10) {
        DDS_HEADER_DX10 headerDX10;
        fs.read(reinterpret_cast<char*>(&headerDX10), sizeof(headerDX10));
    }

    *outWidth = static_cast<uint32_t>(header.width);
    *outHeight = static_cast<uint32_t>(header.height);
    *bytesPerPixel = 4;
    *channels = 4;
    uint32_t dataSize = header.width * header.height * (*bytesPerPixel);

    *outData = new uint8_t[dataSize];
    fs.read(reinterpret_cast<char*>(*outData), dataSize);

    fs.close();
    return true;
}
