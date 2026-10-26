#ifdef ME_METAL

#include "shader_metal.h"

#include <dispatch/dispatch.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

ME::ShaderMetal::ShaderMetal(MTL4::Compiler* compiler, const char* shaderName) {
    // The cook compiles shaders/metal/<name>.metal into <name>.metallib next to where the source would go.
    std::string path = std::string("shaders/metal/") + shaderName;
    const size_t dot = path.find_last_of('.');
    path = (dot == std::string::npos ? path : path.substr(0, dot)) + ".metallib";

    size_t size = 0;
    if (!Vfs::GetFileSize(FileRoot::Resources, path.c_str(), size) || size == 0) {
        ME::LogError("ShaderMetal: can't find ", path);
        return;
    }
    uint8_t* bytes = new uint8_t[size];
    if (!Vfs::ReadBytes(FileRoot::Resources, path.c_str(), bytes, size, size)) {
        ME::LogError("ShaderMetal: can't read ", path);
        delete[] bytes;
        return;
    }

    // The default destructor makes dispatch copy the bytes, so the buffer can go at once.
    dispatch_data_t data = dispatch_data_create(bytes, size, nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    delete[] bytes;

    NS::Error* error = nullptr;
    library = compiler->device()->newLibrary(data, &error);
    dispatch_release(data);
    if (library == nullptr) {
        ME::LogError("ShaderMetal: ", path, " is not a valid library: ",
                     error != nullptr ? error->localizedDescription()->utf8String() : "unknown error");
    }
}

ME::ShaderMetal::~ShaderMetal() {
    if (library != nullptr) {
        library->release();
        library = nullptr;
    }
}

MTL::Library* ME::ShaderMetal::GetLibrary() const {
    return library;
}

#endif  // ME_METAL
