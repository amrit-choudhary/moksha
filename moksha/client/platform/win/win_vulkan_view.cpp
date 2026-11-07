#if defined(ME_WIN) && defined(ME_VULKAN)

#include "win_vulkan_view.h"

// After windows.h (pulled in by the header), which it needs for HWND and HINSTANCE.
#include <vulkan/vulkan_win32.h>

#include "moksha/logging/logging.h"

namespace {

const char* const kInstanceExtensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

}  // namespace

uint32_t ME::WinVulkanView::GetInstanceExtensionCount() const {
    return static_cast<uint32_t>(sizeof(kInstanceExtensions) / sizeof(kInstanceExtensions[0]));
}

const char* const* ME::WinVulkanView::GetInstanceExtensions() const {
    return kInstanceExtensions;
}

bool ME::WinVulkanView::CreateSurface(VkInstance vkInstance, VkSurfaceKHR* outSurface) const {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = instance;
    createInfo.hwnd = window;
    const VkResult result = vkCreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, outSurface);
    if (result != VK_SUCCESS) {
        ME::LogError("WinVulkanView: vkCreateWin32SurfaceKHR failed (", static_cast<int>(result), ")");
        return false;
    }
    return true;
}

uint32_t ME::WinVulkanView::GetPixelWidth() const {
    RECT rect = {};
    GetClientRect(window, &rect);
    return static_cast<uint32_t>(rect.right - rect.left);
}

uint32_t ME::WinVulkanView::GetPixelHeight() const {
    RECT rect = {};
    GetClientRect(window, &rect);
    return static_cast<uint32_t>(rect.bottom - rect.top);
}

#endif  // ME_WIN && ME_VULKAN
