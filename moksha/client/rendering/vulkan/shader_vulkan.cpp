#ifdef ME_VULKAN

#include "shader_vulkan.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

namespace {

// Reads a cooked .spv from Resources into a new module. False (logged) if it is missing, not SPIR-V or rejected.
bool ReadShaderModule(VkDevice device, const std::string& path, VkShaderModule* outModule) {
    *outModule = VK_NULL_HANDLE;

    // SPIR-V is a stream of 32-bit words.
    size_t size = 0;
    if (!ME::Vfs::GetFileSize(ME::FileRoot::Resources, path.c_str(), size) || size == 0 || size % 4 != 0) {
        ME::LogError("ShaderVulkan: ", path, " is missing or not SPIR-V");
        return false;
    }

    // uint32_t storage keeps pCode aligned, as Vulkan requires.
    uint32_t* words = new uint32_t[size / 4];
    size_t readSize = 0;
    if (!ME::Vfs::ReadBytes(ME::FileRoot::Resources, path.c_str(), reinterpret_cast<uint8_t*>(words), size, readSize)) {
        ME::LogError("ShaderVulkan: can't read ", path);
        delete[] words;
        return false;
    }

    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = size;  // in bytes
    info.pCode = words;
    const VkResult result = vkCreateShaderModule(device, &info, nullptr, outModule);
    delete[] words;  // Vulkan copied the code
    if (result < 0) {
        ME::LogError("ShaderVulkan: vkCreateShaderModule failed for ", path, " (", static_cast<int>(result), ")");
        return false;
    }
    return true;
}

}  // namespace

bool ME::ShaderVulkan::Init(VkDevice inDevice, const char* shaderName) {
    device = inDevice;

    const std::string base = std::string("shaders/glsl/") + shaderName;
    if (!ReadShaderModule(device, base + ".vert.spv", &vertModule)) {
        return false;
    }
    if (!ReadShaderModule(device, base + ".frag.spv", &fragModule)) {
        return false;
    }
    return true;
}

void ME::ShaderVulkan::End() {
    if (vertModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, vertModule, nullptr);
        vertModule = VK_NULL_HANDLE;
    }
    if (fragModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, fragModule, nullptr);
        fragModule = VK_NULL_HANDLE;
    }
}

VkShaderModule ME::ShaderVulkan::GetVertModule() const {
    return vertModule;
}

VkShaderModule ME::ShaderVulkan::GetFragModule() const {
    return fragModule;
}

#endif  // ME_VULKAN
