#ifdef ME_WEBGPU

#include "renderer_webgpu.h"

#include "moksha/logging/logging.h"
#include "pso_webgpu.h"
#include "webgpu_view.h"

// Same clear colour as the other renderers.
static const WGPUColor kClearColor = {0.01, 0.01, 0.01, 1.0};

// A canvas can't be configured with an sRGB format directly, only viewed through one. Undefined if there is none.
static WGPUTextureFormat ToSrgbFormat(WGPUTextureFormat format) {
    switch (format) {
        case WGPUTextureFormat_BGRA8Unorm:
            return WGPUTextureFormat_BGRA8UnormSrgb;
        case WGPUTextureFormat_RGBA8Unorm:
            return WGPUTextureFormat_RGBA8UnormSrgb;
        default:
            return WGPUTextureFormat_Undefined;
    }
}

bool ME::RendererWebGPU::InitWebGPU(const WebGPUView* inView, const WindowState* inWindow) {
    view = inView;
    window = inWindow;
    device = view->GetDevice();
    queue = wgpuDeviceGetQueue(device);

    WGPUSurfaceCapabilities capabilities = WGPU_SURFACE_CAPABILITIES_INIT;
    if (wgpuSurfaceGetCapabilities(view->GetSurface(), view->GetAdapter(), &capabilities) != WGPUStatus_Success ||
        capabilities.formatCount == 0) {
        ME::LogError("RendererWebGPU: the surface reports no formats");
        return false;
    }
    const WGPUTextureFormat surfaceFormat = capabilities.formats[0];
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);

    renderFormat = ToSrgbFormat(surfaceFormat);
    if (renderFormat == WGPUTextureFormat_Undefined) {
        ME::LogError("RendererWebGPU: the surface's format has no sRGB view");
        return false;
    }

    WGPUSurfaceConfiguration configuration = WGPU_SURFACE_CONFIGURATION_INIT;
    configuration.device = device;
    configuration.format = surfaceFormat;
    configuration.usage = WGPUTextureUsage_RenderAttachment;
    configuration.width = view->GetPixelWidth();
    configuration.height = view->GetPixelHeight();
    configuration.viewFormatCount = 1;
    configuration.viewFormats = &renderFormat;
    configuration.alphaMode = WGPUCompositeAlphaMode_Opaque;
    configuration.presentMode = WGPUPresentMode_Fifo;
    wgpuSurfaceConfigure(view->GetSurface(), &configuration);

    passLayout = PSOWebGPU::CreatePassLayout(device);
    textureLayout = PSOWebGPU::CreateTextureLayout(device);
    perSpriteLayout = PSOWebGPU::CreatePerSpriteLayout(device);
    instanceLayout = PSOWebGPU::CreateInstanceLayout(device);
    perSpritePipelineLayout = PSOWebGPU::CreatePipelineLayout(device, passLayout, textureLayout, perSpriteLayout);
    instancePipelineLayout = PSOWebGPU::CreatePipelineLayout(device, passLayout, textureLayout, instanceLayout);

    pso2DAtl = PSOWebGPU::CreatePSO2D(device, perSpritePipelineLayout, "sprite_atlas.wgsl", renderFormat, true);
    pso2DInsAtl =
        PSOWebGPU::CreatePSO2D(device, instancePipelineLayout, "sprite_instanced_atlas.wgsl", renderFormat, true);
    pso2DUIText = PSOWebGPU::CreatePSO2D(device, instancePipelineLayout, "ui_text.wgsl", renderFormat, false);
    pso2DUISprite = PSOWebGPU::CreatePSO2D(device, instancePipelineLayout, "ui_sprite.wgsl", renderFormat, false);
    if (pso2DAtl == nullptr || pso2DInsAtl == nullptr || pso2DUIText == nullptr || pso2DUISprite == nullptr) {
        ME::LogError("RendererWebGPU: couldn't create the pipelines");
        return false;
    }

    ME::Log("RendererWebGPU::InitWebGPU");
    return true;
}

void ME::RendererWebGPU::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    scene = gameScene;
    uiScene = inUiScene;
}

void ME::RendererWebGPU::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    debugUiScene = inDebugUiScene;
}

void ME::RendererWebGPU::Update() {
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

void ME::RendererWebGPU::Draw() {
    if (queue == nullptr || renderFormat == WGPUTextureFormat_Undefined) {
        return;
    }

    WGPUSurfaceTexture surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(view->GetSurface(), &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        return;
    }

    WGPUTextureViewDescriptor viewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDescriptor.format = renderFormat;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    WGPURenderPassColorAttachment colorTarget = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    colorTarget.view = targetView;
    colorTarget.loadOp = WGPULoadOp_Clear;
    colorTarget.storeOp = WGPUStoreOp_Store;
    colorTarget.clearValue = kClearColor;

    WGPURenderPassDescriptor renderPass = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    renderPass.colorAttachmentCount = 1;
    renderPass.colorAttachments = &colorTarget;

    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
    WGPURenderPassEncoder encoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPass);
    wgpuRenderPassEncoderEnd(encoder);
    wgpuRenderPassEncoderRelease(encoder);

    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(commandEncoder, nullptr);
    wgpuQueueSubmit(queue, 1, &commands);

    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(commandEncoder);
    wgpuTextureViewRelease(targetView);
    wgpuTextureRelease(surfaceTexture.texture);
}

void ME::RendererWebGPU::End() {
    for (WGPURenderPipeline* pipeline : {&pso2DAtl, &pso2DInsAtl, &pso2DUIText, &pso2DUISprite}) {
        if (*pipeline != nullptr) {
            wgpuRenderPipelineRelease(*pipeline);
            *pipeline = nullptr;
        }
    }
    for (WGPUPipelineLayout* layout : {&perSpritePipelineLayout, &instancePipelineLayout}) {
        if (*layout != nullptr) {
            wgpuPipelineLayoutRelease(*layout);
            *layout = nullptr;
        }
    }
    for (WGPUBindGroupLayout* layout : {&passLayout, &textureLayout, &perSpriteLayout, &instanceLayout}) {
        if (*layout != nullptr) {
            wgpuBindGroupLayoutRelease(*layout);
            *layout = nullptr;
        }
    }
    if (queue != nullptr) {
        wgpuQueueRelease(queue);
        queue = nullptr;
    }
    device = nullptr;
    view = nullptr;
    ME::Log("RendererWebGPU::End");
}

void ME::RendererWebGPU::OnPause() {}

void ME::RendererWebGPU::SetVsyncEnabled(bool /*enabled*/) {}

#endif  // ME_WEBGPU
