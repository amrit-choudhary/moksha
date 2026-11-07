#ifdef ME_VULKAN

#include "renderer_vulkan.h"

#include "moksha/logging/logging.h"
#include "vulkan_view.h"

bool ME::RendererVulkan::InitVulkan(const VulkanView* inView, const WindowState* inWindow) {
    view = inView;
    window = inWindow;
    ME::Log("RendererVulkan::Init (skeleton: nothing is drawn yet)");
    return true;
}

void ME::RendererVulkan::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    scene = gameScene;
    uiScene = inUiScene;
}

void ME::RendererVulkan::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    debugUiScene = inDebugUiScene;
}

void ME::RendererVulkan::Update() {
    if (scene != nullptr) {
        scene->Update();
    }
    if (uiScene != nullptr) {
        uiScene->Update();
    }
    if (debugUiScene != nullptr) {
        debugUiScene->Update();
    }
}

void ME::RendererVulkan::Draw() {}

void ME::RendererVulkan::End() {
    ME::Log("RendererVulkan::End");
}

void ME::RendererVulkan::OnPause() {}

void ME::RendererVulkan::SetVsyncEnabled(bool /*enabled*/) {}

#endif  // ME_VULKAN
