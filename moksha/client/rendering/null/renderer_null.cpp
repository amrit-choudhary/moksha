#include "renderer_null.h"

#include "moksha/logging/logging.h"

void ME::RendererNull::Init() {}

bool ME::RendererNull::InitHeadless(const WindowState* inWindow) {
    window = inWindow;
    ME::Log("RendererNull::Init (headless, nothing is drawn)");
    return true;
}

void ME::RendererNull::SetScene(ME::Scene* gameScene) {
    scene = gameScene;
}

void ME::RendererNull::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    scene = gameScene;
    uiScene = inUiScene;
}

void ME::RendererNull::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    debugUiScene = inDebugUiScene;
}

void ME::RendererNull::Update() {
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

void ME::RendererNull::Draw() {}

void ME::RendererNull::End() {
    ME::Log("RendererNull::End");
}

void ME::RendererNull::SetVsyncEnabled(bool /*enabled*/) {}
