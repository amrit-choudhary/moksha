#ifdef ME_METAL

#include "renderer_metal.h"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "metal_view.h"
#include "moksha/logging/logging.h"

// Same clear colour as the DX12 renderer.
static const MTL::ClearColor kClearColor = MTL::ClearColor::Make(0.01, 0.01, 0.01, 1.0);

void ME::RendererMetal::Init() {
    ME::Log("RendererMetal::Init");
}

void ME::RendererMetal::SetScene(ME::Scene* gameScene) {
    scene = gameScene;
}

void ME::RendererMetal::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    scene = gameScene;
    uiScene = inUiScene;
}

void ME::RendererMetal::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    debugUiScene = inDebugUiScene;
}

void ME::RendererMetal::SetVsyncEnabled(bool enabled) {
    vsyncEnabled = enabled;
    vsyncChanged = true;
}

void ME::RendererMetal::Update() {
    // Nothing is uploaded to the GPU yet; scene state still updates as on every renderer.
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

void ME::RendererMetal::End() {
    if (commandQueue != nullptr) {
        commandQueue->release();
        commandQueue = nullptr;
    }
    if (device != nullptr) {
        device->release();
        device = nullptr;
    }
    view = nullptr;
    ME::Log("RendererMetal::End");
}

bool ME::RendererMetal::InitMetal(const MetalView* inView, const WindowState* inWindow) {
    view = inView;
    window = inWindow;
    if (view == nullptr || view->GetDevice() == nullptr) {
        ME::LogError("RendererMetal: the view has no Metal device");
        return false;
    }
    device = view->GetDevice()->retain();
    commandQueue = device->newCommandQueue();
    ME::Log("RendererMetal::InitMetal");
    return true;
}

void ME::RendererMetal::Draw() {
    if (commandQueue == nullptr) {
        return;
    }

    // Both are nil while the view has no drawable to give (e.g. the window is hidden); skip the frame then.
    MTL::RenderPassDescriptor* renderPass = view->GetCurrentRenderPass();
    CA::MetalDrawable* drawable = view->GetCurrentDrawable();
    if (renderPass == nullptr || drawable == nullptr) {
        return;
    }
    MTL::RenderPassColorAttachmentDescriptor* colorTarget = renderPass->colorAttachments()->object(0);
    colorTarget->setLoadAction(MTL::LoadActionClear);
    colorTarget->setClearColor(kClearColor);
    if (vsyncChanged) {
        drawable->layer()->setDisplaySyncEnabled(vsyncEnabled);
        vsyncChanged = false;
    }

    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    encoder->endEncoding();

    commandBuffer->presentDrawable(drawable);
    commandBuffer->commit();
}

#endif  // ME_METAL
