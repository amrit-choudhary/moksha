#ifdef ME_METAL

#include "renderer_metal.h"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "metal_view.h"
#include "moksha/logging/logging.h"

// Same clear colour as the DX12 renderer.
static const MTL::ClearColor kClearColor = MTL::ClearColor::Make(0.01, 0.01, 0.01, 1.0);

// Longest wait for the GPU before giving up on a frame.
static constexpr uint64_t kGpuWaitMilliseconds = 1000;

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
    if (view != nullptr) {
        view->GetLayer()->setDisplaySyncEnabled(vsyncEnabled);
    }
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
    if (commandQueue != nullptr && fence != nullptr) {
        FlushCommandQueue();
    }
    if (commandBuffer != nullptr) {
        commandBuffer->release();
        commandBuffer = nullptr;
    }
    if (commandAllocator != nullptr) {
        commandAllocator->release();
        commandAllocator = nullptr;
    }
    if (fence != nullptr) {
        fence->release();
        fence = nullptr;
    }
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

    commandQueue = device->newMTL4CommandQueue();
    commandAllocator = device->newCommandAllocator();
    commandBuffer = device->newCommandBuffer();
    fence = device->newSharedEvent();
    if (commandQueue == nullptr || commandAllocator == nullptr || commandBuffer == nullptr || fence == nullptr) {
        ME::LogError("RendererMetal: couldn't create the Metal 4 command objects");
        return false;
    }

    // Metal 4 makes nothing resident by itself; the layer's set keeps its drawables resident for this queue.
    commandQueue->addResidencySet(view->GetLayer()->residencySet());
    view->GetLayer()->setDisplaySyncEnabled(vsyncEnabled);

    ME::Log("RendererMetal::InitMetal");
    return true;
}

void ME::RendererMetal::Draw() {
    if (commandQueue == nullptr || fence == nullptr) {
        return;
    }

    // Both are nil while the view has no drawable to give (e.g. the window is hidden); skip the frame then.
    MTL4::RenderPassDescriptor* renderPass = view->GetCurrentRenderPass();
    CA::MetalDrawable* drawable = view->GetCurrentDrawable();
    if (renderPass == nullptr || drawable == nullptr) {
        return;
    }
    MTL::RenderPassColorAttachmentDescriptor* colorTarget = renderPass->colorAttachments()->object(0);
    colorTarget->setLoadAction(MTL::LoadActionClear);
    colorTarget->setClearColor(kClearColor);

    // The previous frame was flushed, so its allocator memory is free to reuse.
    commandAllocator->reset();
    commandBuffer->beginCommandBuffer(commandAllocator);

    MTL4::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    encoder->endEncoding();

    commandBuffer->endCommandBuffer();

    // Rendering waits until the drawable is free; presenting waits until the work that drew it is done.
    const MTL4::CommandBuffer* commandBuffers[] = {commandBuffer};
    commandQueue->wait(drawable);
    commandQueue->commit(commandBuffers, 1);
    commandQueue->signalDrawable(drawable);
    drawable->present();

    FlushCommandQueue();
}

void ME::RendererMetal::FlushCommandQueue() {
    ++currentFence;
    commandQueue->signalEvent(fence, currentFence);
    if (!fence->waitUntilSignaledValue(currentFence, kGpuWaitMilliseconds)) {
        ME::LogError("RendererMetal: the GPU didn't finish a frame in time");
    }
}

#endif  // ME_METAL
