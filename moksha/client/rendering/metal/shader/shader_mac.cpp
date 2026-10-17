#ifdef ME_METAL

#include "shader_mac.h"

#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

ME::Shader::Shader(MTL::Device* device, const char* shaderName)
    : device(device), library(nullptr), vertexFunction(nullptr), fragmentFunction(nullptr) {
    std::string shaderString;
    if (!ME::Vfs::ReadText(ME::FileRoot::Resources, shaderName, shaderString)) {
        ME::LogError("Failed to open shader file:", shaderName);
        return;
    }

    NS::Error* error = nullptr;
    using NS::StringEncoding::UTF8StringEncoding;
    library = device->newLibrary(NS::String::string(shaderString.c_str(), NS::StringEncoding::UTF8StringEncoding),
                                 nullptr, &error);
    if (error) {
        __builtin_printf("Error creating Metal library: %s", error->localizedDescription()->utf8String());
        return;
    }

    vertexFunction = library->newFunction(NS::String::string("vertexMain", NS::StringEncoding::UTF8StringEncoding));
    fragmentFunction = library->newFunction(NS::String::string("fragmentMain", NS::StringEncoding::UTF8StringEncoding));
}

ME::Shader::~Shader() {
    if (vertexFunction) {
        vertexFunction->release();
    }
    if (fragmentFunction) {
        fragmentFunction->release();
    }
    if (library) {
        library->release();
    }
}

MTL::Function* ME::Shader::GetVertexFunction() const {
    return vertexFunction;
}

MTL::Function* ME::Shader::GetFragmentFunction() const {
    return fragmentFunction;
}

#endif  // ME_METAL