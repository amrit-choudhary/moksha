#ifdef ME_METAL

#include "shader_metal.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

ME::ShaderMetal::ShaderMetal(MTL4::Compiler* compiler, const char* shaderName) {
    const std::string path = std::string("shaders/metal/") + shaderName;
    std::string source;
    if (!Vfs::ReadText(FileRoot::Resources, path.c_str(), source)) {
        ME::LogError("ShaderMetal: can't read ", path);
        return;
    }

    MTL4::LibraryDescriptor* descriptor = MTL4::LibraryDescriptor::alloc()->init();
    descriptor->setName(NS::String::string(shaderName, NS::UTF8StringEncoding));
    descriptor->setSource(NS::String::string(source.c_str(), NS::UTF8StringEncoding));

    NS::Error* error = nullptr;
    library = compiler->newLibrary(descriptor, &error);
    descriptor->release();
    if (library == nullptr) {
        ME::LogError("ShaderMetal: ", path, " failed to compile: ",
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
