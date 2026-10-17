#ifdef ME_MAC

#include "shader_mac.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "moksha/shared/misc/utils.h"

ME::Shader::Shader(MTL::Device* device, const char* shaderName)
    : device(device), library(nullptr), vertexFunction(nullptr), fragmentFunction(nullptr) {
    std::string filePath = ME::GetResourcesPath() + shaderName;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string shaderString = buffer.str();
    file.close();

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

#endif  // ME_MAC