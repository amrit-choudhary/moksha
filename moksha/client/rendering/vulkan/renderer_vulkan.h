#pragma once
#ifdef ME_VULKAN

/**
 * Renderer class for Vulkan (Android; on Windows a development-only build). Reaches the window only through
 * VulkanView, so the same code runs on every OS. In progress: it creates the device and swapchain, updates the
 * scenes, and draws nothing yet.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"
#include "moksha/shared/misc/engine_limits.h"

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
    bool validationEnabled = false;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = UINT32_MAX;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = {};
    // Drivers make minImageCount + 1 or more (often 3-4 on Android); 8 leaves room.
    static constexpr uint32_t kMaxSwapchainImages = 8;
    VkImage swapchainImages[kMaxSwapchainImages] = {};
    uint32_t swapchainImageCount = 0;
    VkImageView swapchainImageViews[kMaxSwapchainImages] = {};
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer swapchainFramebuffers[kMaxSwapchainImages] = {};
    VkCommandPool commandPools[Constants::FramesInFlight] = {};
    VkCommandBuffer commandBuffers[Constants::FramesInFlight] = {};
    uint32_t frameIndex = 0;
    VkSemaphore imageAvailableSemaphores[Constants::FramesInFlight] = {};
    VkFence inFlightFences[Constants::FramesInFlight] = {};
    VkSemaphore renderFinishedSemaphores[kMaxSwapchainImages] = {};

    bool CreateInstance();
    bool SetupDebugMessenger();
    bool PickPhysicalDevice();
    bool CreateSurface();
    bool PickQueueFamily();
    bool CreateLogicalDevice();
    bool CreateSwapchain();
    bool CreateSwapchainImageViews();
    bool CreateRenderPass();
    bool CreateSwapchainFramebuffers();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();
};

}  // namespace ME

#endif  // ME_VULKAN
