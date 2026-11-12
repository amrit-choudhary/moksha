#pragma once
#ifdef ME_VULKAN

/**
 * Renderer class for Vulkan (Android; on Windows a development-only build). Reaches the window only through
 * VulkanView, so the same code runs on every OS. Draws what RendererDX draws: the game scene's plain and instanced
 * sprites (SceneVulkan), then the game's UI and the debug overlay (SceneUIVulkan each). Swapchain recreation is
 * still missing.
 */

#include <vulkan/vulkan.h>

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"
#include "moksha/shared/misc/engine_limits.h"
#include "quad_vulkan.h"

namespace ME {

class Scene;
class SceneVulkan;
class SceneUI;
class SceneUIVulkan;
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
    ME::SceneVulkan* sceneVulkan = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUIVulkan* uiSceneVulkan = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    ME::SceneUIVulkan* debugUiSceneVulkan = nullptr;

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
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
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

    ME::QuadVulkan quad;

    // Shared samplers, built into the set layouts as DX12's static samplers are; destroyed after the layouts.
    VkSampler samplerPointClamp = VK_NULL_HANDLE;

    // Descriptor set layouts (set 0 per pass, set 1 per sprite or instanced, set 2 texture), shared by the pipeline
    // layouts and the sets.
    VkDescriptorSetLayout setLayoutPerPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayoutPerSprite = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayoutInstanced = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayoutTexture = VK_NULL_HANDLE;

    // Pipelines and their layouts, as RendererDX's PSOs and root signatures.
    VkPipelineLayout layout2DAtl = VK_NULL_HANDLE;
    VkPipeline pso2DAtl = VK_NULL_HANDLE;
    VkPipelineLayout layout2DInsAtl = VK_NULL_HANDLE;
    VkPipeline pso2DInsAtl = VK_NULL_HANDLE;
    VkPipeline pso2DInsAtlAdditive = VK_NULL_HANDLE;  // pso2DInsAtl for sets set to SpriteBlend::Additive
    VkPipelineLayout layout2DUISprite = VK_NULL_HANDLE;
    VkPipeline pso2DUISprite = VK_NULL_HANDLE;
    VkPipelineLayout layout2DUIText = VK_NULL_HANDLE;
    VkPipeline pso2DUIText = VK_NULL_HANDLE;

    // One-time upload commands (the scenes' textures).
    VkCommandPool uploadCommandPool = VK_NULL_HANDLE;

    bool CreateInstance();
    bool SetupDebugMessenger();
    bool PickPhysicalDevice();
    bool CreateSurface();
    bool PickQueueFamily();
    bool CreateLogicalDevice();
    bool CreateSwapchain();
    bool CreateSwapchainImageViews();
    bool PickDepthFormat();
    bool CreateDepthResources();
    bool CreateRenderPass();
    bool CreateSwapchainFramebuffers();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();
    bool CreatePSO();

    // Draws the scene's plain sprites: sets 1 and 2 per sprite. Set 0 must already be bound.
    void DrawSprites(VkCommandBuffer cmd);

    // Draws one instanced set in a single call with texture and atlas `slot`, blended as blend says; skipped if the set
    // is empty or the slot is out of range or failed to load. Set 0 must already be bound.
    void DrawInstancedSprites(VkCommandBuffer cmd, VkDescriptorSet instanceSet, uint32_t instanceOffset,
                              uint32_t instanceCount, uint32_t slot, ME::SpriteBlend blend);

    // Creates a UI scene's GPU resources; null (logged) on failure. The device must be idle if one is replaced.
    ME::SceneUIVulkan* CreateUIScene(ME::SceneUI* sceneUI);
    // Ends and deletes *uiSceneGPU, then nulls it. The GPU must be done with it.
    void DestroyUIScene(ME::SceneUIVulkan** uiSceneGPU);

    // Draws a UI scene over the frame: UI sprites with texture and atlas slot 0, then text with slot 1, each pass
    // skipped if it is empty or its slot is missing. Binds its own set 0.
    void DrawUIScene(VkCommandBuffer cmd, const ME::SceneUIVulkan* uiSceneGPU);
};

}  // namespace ME

#endif  // ME_VULKAN
