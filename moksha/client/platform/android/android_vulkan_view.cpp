#if defined(ME_ANDROID) && defined(ME_VULKAN)

#include "android_vulkan_view.h"

// After android/native_window.h (pulled in by the header), which it needs for ANativeWindow.
#include <vulkan/vulkan_android.h>

#include "moksha/logging/logging.h"

namespace {

const char* const kInstanceExtensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};

}  // namespace

uint32_t ME::AndroidVulkanView::GetInstanceExtensionCount() const {
    return static_cast<uint32_t>(sizeof(kInstanceExtensions) / sizeof(kInstanceExtensions[0]));
}

const char* const* ME::AndroidVulkanView::GetInstanceExtensions() const {
    return kInstanceExtensions;
}

bool ME::AndroidVulkanView::CreateSurface(VkInstance vkInstance, VkSurfaceKHR* outSurface) const {
    if (window == nullptr) {
        ME::LogError("AndroidVulkanView: no window");
        return false;
    }
    VkAndroidSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = window;
    const VkResult result = vkCreateAndroidSurfaceKHR(vkInstance, &createInfo, nullptr, outSurface);
    if (result != VK_SUCCESS) {
        ME::LogError("AndroidVulkanView: vkCreateAndroidSurfaceKHR failed (", static_cast<int>(result), ")");
        return false;
    }
    return true;
}

uint32_t ME::AndroidVulkanView::GetPixelWidth() const {
    return window != nullptr ? static_cast<uint32_t>(ANativeWindow_getWidth(window)) : 0;
}

uint32_t ME::AndroidVulkanView::GetPixelHeight() const {
    return window != nullptr ? static_cast<uint32_t>(ANativeWindow_getHeight(window)) : 0;
}

#endif  // ME_ANDROID && ME_VULKAN
