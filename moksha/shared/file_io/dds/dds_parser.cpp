#include "dds_parser.h"

#include <cstring>
#include <iostream>

#include "moksha/shared/file_io/vfs.h"

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
    size_t fileSize = 0;
    if (!Vfs::GetFileSize(FileRoot::Resources, fileName, fileSize)) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }
    uint8_t* file = new uint8_t[fileSize];
    if (!Vfs::ReadBytes(FileRoot::Resources, fileName, file, fileSize, fileSize)) {
        std::cerr << "Failed to read file: " << fileName << std::endl;
        delete[] file;
        return false;
    }

    // Header fields are copied out of the buffer; the file bytes have no alignment guarantee.
    size_t offset = 0;
    uint32_t magic = 0;
    if (fileSize < sizeof(magic) + sizeof(DDS_HEADER)) {
        std::cerr << "Not a valid DDS file: " << fileName << std::endl;
        delete[] file;
        return false;
    }
    std::memcpy(&magic, file, sizeof(magic));
    offset += sizeof(magic);
    if (magic != FOURCC_DDS) {
        std::cerr << "Not a valid DDS file: " << fileName << std::endl;
        delete[] file;
        return false;
    }

    DDS_HEADER header;
    std::memcpy(&header, file + offset, sizeof(header));
    offset += sizeof(header);

    if (header.format.fourCC == FOURCC_DX10) {
        offset += sizeof(DDS_HEADER_DX10);
    }

    *outWidth = static_cast<uint32_t>(header.width);
    *outHeight = static_cast<uint32_t>(header.height);
    *bytesPerPixel = 4;
    *channels = 4;
    const size_t dataSize = static_cast<size_t>(header.width) * header.height * (*bytesPerPixel);
    if (offset > fileSize || fileSize - offset < dataSize) {
        std::cerr << "DDS file is shorter than its pixel data: " << fileName << std::endl;
        delete[] file;
        return false;
    }

    *outData = new uint8_t[dataSize];
    std::memcpy(*outData, file + offset, dataSize);

    delete[] file;
    return true;
}
