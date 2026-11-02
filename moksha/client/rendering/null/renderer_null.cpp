#include "renderer_null.h"

#include "moksha/logging/logging.h"

bool ME::RendererNull::InitNull(const WindowState* inWindow) {
    window = inWindow;
    ME::Log("RendererNull::Init (nothing is drawn)");
    return true;
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

void ME::RendererNull::OnPause() {}

void ME::RendererNull::SetVsyncEnabled(bool /*enabled*/) {}
