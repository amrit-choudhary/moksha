#pragma once
#ifdef ME_VULKAN

/**
 * What the Vulkan renderer needs from the platform's window: the instance extensions its surface needs, and the
 * surface itself. Each OS implements it over its own window (platform/win: an HWND), so the renderer stays OS-neutral.
 * Not owned; it must outlive the renderer.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ME {

class VulkanView {
   public:
    virtual ~VulkanView() = default;

    // Instance extensions this OS's surface needs (VK_KHR_surface and the OS's own), for VkInstanceCreateInfo. The
    // array stays valid for the view's lifetime.
    virtual uint32_t GetInstanceExtensionCount() const = 0;
    virtual const char* const* GetInstanceExtensions() const = 0;

    // Creates a surface for the window on instance, which must have the extensions above enabled. The caller owns
    // the surface and destroys it with vkDestroySurfaceKHR. False on failure.
    virtual bool CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface) const = 0;

    // The window's drawable size in pixels, for when the surface leaves the swapchain extent to the app.
    virtual uint32_t GetPixelWidth() const = 0;
    virtual uint32_t GetPixelHeight() const = 0;
};

}  // namespace ME

#endif  // ME_VULKAN
