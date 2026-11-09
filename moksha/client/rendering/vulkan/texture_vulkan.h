#pragma once
#ifdef ME_VULKAN

/**
 * Vulkan's texture class, the counterpart of TextureDX: loads a cooked .dds into a device-local sRGB RGBA8 image
 * (optimal tiling), filled through a staging copy.
 */

#include <vulkan/vulkan.h>

#include "moksha/client/rendering/shared/texture.h"

namespace ME {

class TextureVulkan : public Texture {
   public:
    TextureVulkan() = default;
    ~TextureVulkan() override = default;

    TextureVulkan(const TextureVulkan&) = delete;
    TextureVulkan& operator=(const TextureVulkan&) = delete;

    // Loads path (Resources root) and creates the image. False (logged) on failure; call End() either way.
    bool Init(VkPhysicalDevice physicalDevice, VkDevice inDevice, VkQueue queue, VkCommandPool uploadPool,
              const char* path);

    // The GPU must be done with the texture (call after vkDeviceWaitIdle).
    void End();

    VkImage GetImage() const;
    VkImageView GetImageView() const;

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;

    bool Upload(VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool uploadPool);
};

}  // namespace ME

#endif  // ME_VULKAN
