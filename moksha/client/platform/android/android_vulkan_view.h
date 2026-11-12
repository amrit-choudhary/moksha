#pragma once
#if defined(ME_ANDROID) && defined(ME_VULKAN)

/**
 * VulkanView over the NativeActivity's window. The driver sets the window once the OS hands it over
 * (APP_CMD_INIT_WINDOW), before ClientApp::Init; it is only valid until APP_CMD_TERM_WINDOW.
 */

#include <android/native_window.h>

#include <cstdint>

#include "moksha/client/rendering/vulkan/vulkan_view.h"

namespace ME {

class AndroidVulkanView : public VulkanView {
   public:
    uint32_t GetInstanceExtensionCount() const override;
    const char* const* GetInstanceExtensions() const override;
    bool CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface) const override;
    uint32_t GetPixelWidth() const override;
    uint32_t GetPixelHeight() const override;

    ANativeWindow* window = nullptr;
};

}  // namespace ME

#endif  // ME_ANDROID && ME_VULKAN
