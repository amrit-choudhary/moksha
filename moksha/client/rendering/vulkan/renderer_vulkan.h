#pragma once
#ifdef ME_VULKAN

/**
 * Renderer class for Vulkan (Android; on Windows a development-only build). Reaches the window only through
 * VulkanView, so the same code runs on every OS. Skeleton for now: it updates the scenes and draws nothing.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"

namespace ME {

class Scene;
class SceneUI;
class VulkanView;
class WindowState;

class RendererVulkan : public PlatformRenderer {
   public:
    // Sets up Vulkan for the view's window. view must outlive the renderer; window is the app's window state.
    bool InitVulkan(const VulkanView* inView, const WindowState* inWindow);

    void SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) override;
    void SetDebugUIScene(ME::SceneUI* inDebugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    void Draw() override;

    void End() override;

    void OnPause() override;

    void SetVsyncEnabled(bool enabled) override;

   private:
    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;

    const VulkanView* view = nullptr;
    const WindowState* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    bool validationEnabled = false;

    bool CreateInstance();
    bool SetupDebugMessenger();
    bool PickPhysicalDevice();
};

}  // namespace ME

#endif  // ME_VULKAN
