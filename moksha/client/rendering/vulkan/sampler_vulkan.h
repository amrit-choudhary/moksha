#pragma once
#ifdef ME_VULKAN

/**
 * Samplers for Vulkan, the counterpart of RootSigDx's static samplers: one named factory per sampler the shaders use.
 * The caller owns each sampler and destroys it with vkDestroySampler, after every set layout that uses it.
 */

#include <vulkan/vulkan.h>

namespace ME {

class SamplerVulkan {
   public:
    SamplerVulkan() = delete;

    // Nearest filtering, clamp to edge: the sprite and UI sampler (DX12's s1). Null on failure (logged).
    static VkSampler CreatePointClamp(VkDevice device);

   private:
    // No mipmaps, no anisotropy (the feature isn't enabled), no compare. Null on failure (logged).
    static VkSampler Create(VkDevice device, VkFilter filter, VkSamplerAddressMode address);
};

}  // namespace ME

#endif  // ME_VULKAN
