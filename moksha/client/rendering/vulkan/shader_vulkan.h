#pragma once
#ifdef ME_VULKAN

/**
 * Shader class for Vulkan, the counterpart of the DX12 Shader: a vertex and a fragment module from the SPIR-V the cook
 * compiled from shaders/glsl/<name>.vert and .frag (Resources, through Vfs). Modules are only needed while pipelines
 * are created from them.
 */

#include <vulkan/vulkan.h>

namespace ME {

class ShaderVulkan {
   public:
    ShaderVulkan() = default;
    ~ShaderVulkan() = default;

    ShaderVulkan(const ShaderVulkan&) = delete;
    ShaderVulkan& operator=(const ShaderVulkan&) = delete;

    // Loads shaders/glsl/<shaderName>.vert.spv and .frag.spv (shaderName without extension). False if either is
    // missing or invalid (logged); call End() either way to free what was loaded.
    bool Init(VkDevice inDevice, const char* shaderName);
    void End();

    // The modules; null until Init succeeds.
    VkShaderModule GetVertModule() const;
    VkShaderModule GetFragModule() const;

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkShaderModule vertModule = VK_NULL_HANDLE;
    VkShaderModule fragModule = VK_NULL_HANDLE;
};

}  // namespace ME

#endif  // ME_VULKAN
