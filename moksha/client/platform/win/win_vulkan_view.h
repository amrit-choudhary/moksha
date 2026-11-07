#pragma once
#if defined(ME_WIN) && defined(ME_VULKAN)

/**
 * VulkanView over a Win32 window (development-only Vulkan build). The driver fills in the instance and window once the
 * window exists, before ClientApp::Init.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cstdint>

#include "moksha/client/rendering/vulkan/vulkan_view.h"

namespace ME {

class WinVulkanView : public VulkanView {
   public:
    uint32_t GetInstanceExtensionCount() const override;
    const char* const* GetInstanceExtensions() const override;
    bool CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface) const override;
    uint32_t GetPixelWidth() const override;
    uint32_t GetPixelHeight() const override;

    HINSTANCE instance = nullptr;
    HWND window = nullptr;
};

}  // namespace ME

#endif  // ME_WIN && ME_VULKAN
