#ifdef ME_WEBGPU

#include "shader_webgpu.h"

#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

ME::ShaderWebGPU::ShaderWebGPU(WGPUDevice device, const char* shaderName) {
    const std::string path = std::string("shaders/wgsl/") + shaderName;
    std::string code;
    if (!Vfs::ReadText(FileRoot::Resources, path.c_str(), code)) {
        ME::LogError("ShaderWebGPU: can't read ", path);
        return;
    }

    WGPUShaderSourceWGSL source = WGPU_SHADER_SOURCE_WGSL_INIT;
    source.code = {code.c_str(), code.size()};
    WGPUShaderModuleDescriptor descriptor = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    descriptor.nextInChain = &source.chain;
    descriptor.label = {shaderName, WGPU_STRLEN};
    module = wgpuDeviceCreateShaderModule(device, &descriptor);
}

ME::ShaderWebGPU::~ShaderWebGPU() {
    if (module != nullptr) {
        wgpuShaderModuleRelease(module);
        module = nullptr;
    }
}

WGPUShaderModule ME::ShaderWebGPU::GetModule() const {
    return module;
}

#endif  // ME_WEBGPU
