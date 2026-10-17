#include "texture.h"

#include <iostream>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/dds/dds_parser.h"

ME::Texture::Texture() {}

ME::Texture::Texture(const char* path) {
    Load(path);
}

ME::Texture::~Texture() {
    delete[] data;
}

void ME::Texture::Load(const char* path) {
    DDSParser::LoadDDS(path, &data, &width, &height, &bytesPerPixel, &channels);
}
