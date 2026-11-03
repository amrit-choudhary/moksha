#pragma once
#ifdef ME_WEBGPU

/**
 * A shader module from a cooked WGSL file (shaders/wgsl/<name>), compiled by the browser. Compile errors arrive later
 * through the device's error callback.
 */

#include <webgpu/webgpu.h>

namespace ME {

class ShaderWebGPU {
   public:
    ShaderWebGPU(WGPUDevice device, const char* shaderName);
    ~ShaderWebGPU();

    ShaderWebGPU(const ShaderWebGPU&) = delete;
    ShaderWebGPU& operator=(const ShaderWebGPU&) = delete;

    // Null if the file couldn't be read (logged).
    WGPUShaderModule GetModule() const;

   private:
    WGPUShaderModule module = nullptr;
};

}  // namespace ME

#endif  // ME_WEBGPU
