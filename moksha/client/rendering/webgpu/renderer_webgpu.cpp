#ifdef ME_WEBGPU

#include "renderer_webgpu.h"

#include <initializer_list>

#include "moksha/client/app/app_config.h"
#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/light.h"
#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/scene/scene_ui_webgpu.h"
#include "moksha/client/scene/scene_webgpu.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"
#include "quad_webgpu.h"
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
    context.device = view->GetDevice();
    context.queue = wgpuDeviceGetQueue(context.device);

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
    configuration.device = context.device;
    configuration.format = surfaceFormat;
    configuration.usage = WGPUTextureUsage_RenderAttachment;
    configuration.width = view->GetPixelWidth();
    configuration.height = view->GetPixelHeight();
    configuration.viewFormatCount = 1;
    configuration.viewFormats = &renderFormat;
    configuration.alphaMode = WGPUCompositeAlphaMode_Opaque;
    configuration.presentMode = WGPUPresentMode_Fifo;
    wgpuSurfaceConfigure(view->GetSurface(), &configuration);

    context.passLayout = PSOWebGPU::CreatePassLayout(context.device);
    context.textureLayout = PSOWebGPU::CreateTextureLayout(context.device);
    context.perSpriteLayout = PSOWebGPU::CreatePerSpriteLayout(context.device);
    context.instanceLayout = PSOWebGPU::CreateInstanceLayout(context.device);

    WGPUSamplerDescriptor samplerDescriptor = WGPU_SAMPLER_DESCRIPTOR_INIT;
    samplerDescriptor.addressModeU = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.addressModeV = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.addressModeW = WGPUAddressMode_ClampToEdge;
    samplerDescriptor.magFilter = WGPUFilterMode_Nearest;
    samplerDescriptor.minFilter = WGPUFilterMode_Nearest;
    samplerDescriptor.mipmapFilter = WGPUMipmapFilterMode_Nearest;
    context.sampler = wgpuDeviceCreateSampler(context.device, &samplerDescriptor);

    perSpritePipelineLayout = PSOWebGPU::CreatePipelineLayout(context.device, context.passLayout, context.textureLayout,
                                                              context.perSpriteLayout);
    instancePipelineLayout = PSOWebGPU::CreatePipelineLayout(context.device, context.passLayout, context.textureLayout,
                                                             context.instanceLayout);

    pso2DAtl = PSOWebGPU::CreatePSO2D(context.device, perSpritePipelineLayout, "sprite_atlas.wgsl", renderFormat, true);
    pso2DInsAtl = PSOWebGPU::CreatePSO2D(context.device, instancePipelineLayout, "sprite_instanced_atlas.wgsl",
                                         renderFormat, true);
    pso2DUIText = PSOWebGPU::CreatePSO2D(context.device, instancePipelineLayout, "ui_text.wgsl", renderFormat, false);
    pso2DUISprite =
        PSOWebGPU::CreatePSO2D(context.device, instancePipelineLayout, "ui_sprite.wgsl", renderFormat, false);
    if (pso2DAtl == nullptr || pso2DInsAtl == nullptr || pso2DUIText == nullptr || pso2DUISprite == nullptr) {
        ME::LogError("RendererWebGPU: couldn't create the pipelines");
        return false;
    }

    WGPUTextureDescriptor depthDescriptor = WGPU_TEXTURE_DESCRIPTOR_INIT;
    depthDescriptor.usage = WGPUTextureUsage_RenderAttachment;
    depthDescriptor.size = {view->GetPixelWidth(), view->GetPixelHeight(), 1};
    depthDescriptor.format = PSOWebGPU::DepthFormat;
    depthTexture = wgpuDeviceCreateTexture(context.device, &depthDescriptor);
    depthView = wgpuTextureCreateView(depthTexture, nullptr);

    quad = new QuadWebGPU();
    quad->CreateBuffers(context.device, context.queue);

    ME::Log("RendererWebGPU::InitWebGPU");
    return true;
}

void ME::RendererWebGPU::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    delete sceneWebGPU;
    sceneWebGPU = nullptr;
    scene = gameScene;
    if (context.queue != nullptr && gameScene != nullptr) {
        sceneWebGPU = new ME::SceneWebGPU(context, gameScene);
    }

    delete uiSceneWebGPU;
    uiSceneWebGPU = nullptr;
    uiScene = inUiScene;
    if (context.queue != nullptr && inUiScene != nullptr) {
        uiSceneWebGPU = new ME::SceneUIWebGPU(context, inUiScene);
    }
}

void ME::RendererWebGPU::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    delete debugUiSceneWebGPU;
    debugUiSceneWebGPU = nullptr;
    debugUiScene = inDebugUiScene;
    if (context.queue != nullptr && inDebugUiScene != nullptr) {
        debugUiSceneWebGPU = new ME::SceneUIWebGPU(context, inDebugUiScene);
    }
}

void ME::RendererWebGPU::Update() {
    // If some of the scene render related data changed, update the scene. Drawing is done after this.
    if (scene != nullptr) {
        scene->Update();
    }
    // If UI scene is set, update it too.
    if (uiScene != nullptr) {
        uiScene->Update();
    }
    if (uiSceneWebGPU != nullptr) {
        uiSceneWebGPU->Update();
    }
    // If the debug system's UI scene is set, update it too.
    if (debugUiScene != nullptr) {
        debugUiScene->Update();
    }
    if (debugUiSceneWebGPU != nullptr) {
        debugUiSceneWebGPU->Update();
    }
}

void ME::RendererWebGPU::UploadSceneData() {
    if (sceneWebGPU == nullptr) {
        return;
    }
    const uint32_t atlasCount = static_cast<uint32_t>(sceneWebGPU->textureAtlasProperties.count);
    for (uint32_t i = 0; i < atlasCount; ++i) {
        sceneWebGPU->textureAtlasBuffer->SetElement(i, &sceneWebGPU->textureAtlasProperties[i]);
    }
    sceneWebGPU->textureAtlasBuffer->Upload();

    const bool hasInstances =
        sceneWebGPU->instancedSpriteRenderers0.count != 0 || sceneWebGPU->instancedSpriteRenderers1.count != 0;
    if (sceneWebGPU->perSpriteCount != 0 || hasInstances) {
        CBPerPass perPassData{};
        perPassData.viewMatrix = sceneWebGPU->spriteCamera->GetViewMatrix().GetDataForShader();
        perPassData.projectionMatrix = sceneWebGPU->spriteCamera->GetProjectionMatrix().GetDataForShader();
        perPassData.ambientLightData = sceneWebGPU->ambientLight->GetLightDataAmbient();
        perPassData.directionalLightData = sceneWebGPU->directionalLight->GetLightDataDirectional();
        sceneWebGPU->perPassBuffer->SetElement(0, &perPassData);
        sceneWebGPU->perPassBuffer->Upload();
    }

    if (sceneWebGPU->perSpriteCount != 0) {
        for (uint32_t i = 0; i < sceneWebGPU->perSpriteCount; ++i) {
            CBPerSprite perSpriteData{};
            perSpriteData.modelMatrix = sceneWebGPU->spriteTransforms[i].GetModelMatrix().GetDataForShader();
            perSpriteData.color = sceneWebGPU->spriteRenderers[i].color;
            perSpriteData.atlasIndex = sceneWebGPU->spriteRenderers[i].atlasIndex;
            perSpriteData.flags = sceneWebGPU->spriteRenderers[i].flags;
            sceneWebGPU->perSpriteBuffer->SetElement(i, &perSpriteData);
        }
        sceneWebGPU->perSpriteBuffer->Upload();
    }

    if (sceneWebGPU->spriteInstanceBuffer0 != nullptr) {
        sceneWebGPU->spriteInstanceBuffer0->WriteElements(
            sceneWebGPU->spriteInstanceData0, static_cast<uint32_t>(sceneWebGPU->instancedSpriteRenderers0.count));
    }
    if (sceneWebGPU->spriteInstanceBuffer1 != nullptr) {
        sceneWebGPU->spriteInstanceBuffer1->WriteElements(
            sceneWebGPU->spriteInstanceData1, static_cast<uint32_t>(sceneWebGPU->instancedSpriteRenderers1.count));
    }
}

void ME::RendererWebGPU::UploadUIScene(ME::SceneUIWebGPU* uiSceneGPU) {
    if (uiSceneGPU == nullptr) {
        return;
    }
    const uint32_t atlasCount = static_cast<uint32_t>(uiSceneGPU->textureAtlasProperties.count);
    for (uint32_t i = 0; i < atlasCount; ++i) {
        uiSceneGPU->textureAtlasBuffer->SetElement(i, &uiSceneGPU->textureAtlasProperties[i]);
    }
    uiSceneGPU->textureAtlasBuffer->Upload();

    // UI is drawn over the sprites, in UI space mapped by the window's size.
    const uint32_t screenDimension = ME::Utils::Pack16To32(window->width, window->height);
    CBPerPassUISprite spritePassData{};
    spritePassData.screenDimension = screenDimension;
    uiSceneGPU->uiSpritePassBuffer->SetElement(0, &spritePassData);
    uiSceneGPU->uiSpritePassBuffer->Upload();
    CBPerPassUIText textPassData{};
    textPassData.screenDimension = screenDimension;
    uiSceneGPU->textPassBuffer->SetElement(0, &textPassData);
    uiSceneGPU->textPassBuffer->Upload();

    uiSceneGPU->uiSpriteInstanceBuffer->WriteElements(uiSceneGPU->uiSpriteInstanceData.data,
                                                      static_cast<uint32_t>(uiSceneGPU->uiSpriteInstanceData.count));
    uiSceneGPU->textInstanceBuffer->WriteElements(uiSceneGPU->textInstanceData.data,
                                                  static_cast<uint32_t>(uiSceneGPU->textInstanceData.count));
}

void ME::RendererWebGPU::Draw() {
    if (context.queue == nullptr || sceneWebGPU == nullptr) {
        return;
    }

    WGPUSurfaceTexture surfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(view->GetSurface(), &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        return;
    }

    UploadSceneData();
    UploadUIScene(uiSceneWebGPU);
    UploadUIScene(debugUiSceneWebGPU);

    WGPUTextureViewDescriptor viewDescriptor = WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;
    viewDescriptor.format = renderFormat;
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    WGPURenderPassColorAttachment colorTarget = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    colorTarget.view = targetView;
    colorTarget.loadOp = WGPULoadOp_Clear;
    colorTarget.storeOp = WGPUStoreOp_Store;
    colorTarget.clearValue = kClearColor;

    WGPURenderPassDepthStencilAttachment depthTarget = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
    depthTarget.view = depthView;
    depthTarget.depthLoadOp = WGPULoadOp_Clear;
    depthTarget.depthStoreOp = WGPUStoreOp_Discard;
    depthTarget.depthClearValue = 1.0f;

    WGPURenderPassDescriptor renderPass = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    renderPass.colorAttachmentCount = 1;
    renderPass.colorAttachments = &colorTarget;
    renderPass.depthStencilAttachment = &depthTarget;

    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(context.device, nullptr);
    WGPURenderPassEncoder encoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPass);
    wgpuRenderPassEncoderSetVertexBuffer(encoder, 0, quad->vertexBuffer, 0, quad->verticesSize);
    wgpuRenderPassEncoderSetIndexBuffer(encoder, quad->indexBuffer, WGPUIndexFormat_Uint16, 0, quad->indicesSize);

    DrawSprites(encoder);

    // First Instanced Sprite Drawing (texture and atlas slot 1).
    DrawInstancedSpriteSet(encoder, 1, sceneWebGPU->spriteInstanceBindGroup0,
                           static_cast<uint32_t>(sceneWebGPU->instancedSpriteRenderers0.count));

    // Second Instanced Sprite Drawing (texture and atlas slot 2).
    DrawInstancedSpriteSet(encoder, 2, sceneWebGPU->spriteInstanceBindGroup1,
                           static_cast<uint32_t>(sceneWebGPU->instancedSpriteRenderers1.count));

    // UI Drawing (game's own UI, then the debug system on top of it).
    DrawUIScene(encoder, uiSceneWebGPU);
    DrawUIScene(encoder, debugUiSceneWebGPU);

    wgpuRenderPassEncoderEnd(encoder);
    wgpuRenderPassEncoderRelease(encoder);

    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(commandEncoder, nullptr);
    wgpuQueueSubmit(context.queue, 1, &commands);

    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(commandEncoder);
    wgpuTextureViewRelease(targetView);
    wgpuTextureRelease(surfaceTexture.texture);
}

void ME::RendererWebGPU::DrawSprites(WGPURenderPassEncoder encoder) {
    if (sceneWebGPU->perSpriteCount == 0) {
        return;
    }
    wgpuRenderPassEncoderSetPipeline(encoder, pso2DAtl);

    for (uint32_t i = 0; i < sceneWebGPU->perSpriteCount; ++i) {
        const uint32_t textureIndex = sceneWebGPU->spriteRenderers[i].textureId;
        if (textureIndex >= sceneWebGPU->spriteTextureCount ||
            sceneWebGPU->textureBindGroups[textureIndex] == nullptr) {
            continue;
        }

        const uint32_t atlasOffset =
            sceneWebGPU->textureAtlasBuffer->GetOffset(sceneWebGPU->spriteRenderers[i].textureAtlasPropsId);
        const uint32_t spriteOffset = sceneWebGPU->perSpriteBuffer->GetOffset(i);
        wgpuRenderPassEncoderSetBindGroup(encoder, 0, sceneWebGPU->passBindGroup, 1, &atlasOffset);
        wgpuRenderPassEncoderSetBindGroup(encoder, 1, sceneWebGPU->textureBindGroups[textureIndex], 0, nullptr);
        wgpuRenderPassEncoderSetBindGroup(encoder, 2, sceneWebGPU->perSpriteBindGroup, 1, &spriteOffset);
        wgpuRenderPassEncoderDrawIndexed(encoder, quad->indexCount, 1, 0, 0, 0);
    }
}

void ME::RendererWebGPU::DrawInstancedSpriteSet(WGPURenderPassEncoder encoder, uint32_t textureIndex,
                                                WGPUBindGroup instanceBindGroup, uint32_t instanceCount) {
    if (instanceCount == 0 || textureIndex >= sceneWebGPU->spriteTextureCount ||
        sceneWebGPU->textureBindGroups[textureIndex] == nullptr) {
        return;
    }
    wgpuRenderPassEncoderSetPipeline(encoder, pso2DInsAtl);

    // The texture and its atlas properties share the slot index, as in RendererDX.
    const uint32_t atlasOffset = sceneWebGPU->textureAtlasBuffer->GetOffset(textureIndex);
    wgpuRenderPassEncoderSetBindGroup(encoder, 0, sceneWebGPU->passBindGroup, 1, &atlasOffset);
    wgpuRenderPassEncoderSetBindGroup(encoder, 1, sceneWebGPU->textureBindGroups[textureIndex], 0, nullptr);
    wgpuRenderPassEncoderSetBindGroup(encoder, 2, instanceBindGroup, 0, nullptr);
    wgpuRenderPassEncoderDrawIndexed(encoder, quad->indexCount, instanceCount, 0, 0, 0);
}

void ME::RendererWebGPU::DrawUIScene(WGPURenderPassEncoder encoder, ME::SceneUIWebGPU* uiSceneGPU) {
    if (uiSceneGPU == nullptr) {
        return;
    }

    // UI Sprite Drawing (atlas at texture slot 0).
    if (uiSceneGPU->uiSpriteInstanceData.count != 0 && uiSceneGPU->spriteTextureCount > 0 &&
        uiSceneGPU->textureBindGroups[0] != nullptr) {
        wgpuRenderPassEncoderSetPipeline(encoder, pso2DUISprite);

        const uint32_t atlasOffset = uiSceneGPU->textureAtlasBuffer->GetOffset(0);
        wgpuRenderPassEncoderSetBindGroup(encoder, 0, uiSceneGPU->uiSpritePassBindGroup, 1, &atlasOffset);
        wgpuRenderPassEncoderSetBindGroup(encoder, 1, uiSceneGPU->textureBindGroups[0], 0, nullptr);
        wgpuRenderPassEncoderSetBindGroup(encoder, 2, uiSceneGPU->uiSpriteInstanceBindGroup, 0, nullptr);
        wgpuRenderPassEncoderDrawIndexed(encoder, quad->indexCount,
                                         static_cast<uint32_t>(uiSceneGPU->uiSpriteInstanceData.count), 0, 0, 0);
    }

    // Text Drawing (font at texture slot 1).
    if (uiSceneGPU->textInstanceData.count != 0 && uiSceneGPU->spriteTextureCount > 1 &&
        uiSceneGPU->textureBindGroups[1] != nullptr) {
        wgpuRenderPassEncoderSetPipeline(encoder, pso2DUIText);

        const uint32_t atlasOffset = uiSceneGPU->textureAtlasBuffer->GetOffset(1);
        wgpuRenderPassEncoderSetBindGroup(encoder, 0, uiSceneGPU->textPassBindGroup, 1, &atlasOffset);
        wgpuRenderPassEncoderSetBindGroup(encoder, 1, uiSceneGPU->textureBindGroups[1], 0, nullptr);
        wgpuRenderPassEncoderSetBindGroup(encoder, 2, uiSceneGPU->textInstanceBindGroup, 0, nullptr);
        wgpuRenderPassEncoderDrawIndexed(encoder, quad->indexCount,
                                         static_cast<uint32_t>(uiSceneGPU->textInstanceData.count), 0, 0, 0);
    }
}

void ME::RendererWebGPU::End() {
    delete sceneWebGPU;
    sceneWebGPU = nullptr;
    delete uiSceneWebGPU;
    uiSceneWebGPU = nullptr;
    delete debugUiSceneWebGPU;
    debugUiSceneWebGPU = nullptr;
    delete quad;
    quad = nullptr;

    if (depthView != nullptr) {
        wgpuTextureViewRelease(depthView);
        depthView = nullptr;
    }
    if (depthTexture != nullptr) {
        wgpuTextureRelease(depthTexture);
        depthTexture = nullptr;
    }
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
    for (WGPUBindGroupLayout* layout :
         {&context.passLayout, &context.textureLayout, &context.perSpriteLayout, &context.instanceLayout}) {
        if (*layout != nullptr) {
            wgpuBindGroupLayoutRelease(*layout);
            *layout = nullptr;
        }
    }
    if (context.sampler != nullptr) {
        wgpuSamplerRelease(context.sampler);
        context.sampler = nullptr;
    }
    if (context.queue != nullptr) {
        wgpuQueueRelease(context.queue);
        context.queue = nullptr;
    }
    context.device = nullptr;
    view = nullptr;
    ME::Log("RendererWebGPU::End");
}

void ME::RendererWebGPU::OnPause() {}

void ME::RendererWebGPU::SetVsyncEnabled(bool /*enabled*/) {}

#endif  // ME_WEBGPU
