#ifdef ME_METAL

#include "renderer_metal.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "metal_view.h"
#include "moksha/client/app/app_config.h"
#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/light.h"
#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/scene/scene_metal.h"
#include "moksha/client/scene/scene_ui_metal.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"
#include "pso_metal.h"
#include "quad_metal.h"

// Same clear colour as the DX12 renderer.
static const MTL::ClearColor kClearColor = MTL::ClearColor::Make(0.01, 0.01, 0.01, 1.0);

// Longest wait for the GPU before giving up on a frame.
static constexpr uint64_t kGpuWaitMilliseconds = 1000;

void ME::RendererMetal::Init() {
    ME::Log("RendererMetal::Init");
}

void ME::RendererMetal::SetScene(ME::Scene* gameScene) {
    delete sceneMetal;
    sceneMetal = nullptr;
    scene = gameScene;
    // InitMetal failed: nothing can be created, and Draw does nothing.
    if (residencySet == nullptr || gameScene == nullptr) {
        return;
    }
    sceneMetal = new ME::SceneMetal(device, residencySet, gameScene);
    residencySet->commit();
}

void ME::RendererMetal::SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) {
    SetScene(gameScene);

    delete uiSceneMetal;
    uiSceneMetal = nullptr;
    uiScene = inUiScene;
    if (residencySet == nullptr || inUiScene == nullptr) {
        return;
    }
    uiSceneMetal = new ME::SceneUIMetal(device, residencySet, inUiScene);
    residencySet->commit();
}

void ME::RendererMetal::SetDebugUIScene(ME::SceneUI* inDebugUiScene) {
    delete debugUiSceneMetal;
    debugUiSceneMetal = nullptr;
    debugUiScene = inDebugUiScene;
    if (residencySet == nullptr || inDebugUiScene == nullptr) {
        return;
    }
    debugUiSceneMetal = new ME::SceneUIMetal(device, residencySet, inDebugUiScene);
    residencySet->commit();
}

void ME::RendererMetal::SetVsyncEnabled(bool enabled) {
    vsyncEnabled = enabled;
    if (view != nullptr) {
        view->GetLayer()->setDisplaySyncEnabled(vsyncEnabled);
    }
}

void ME::RendererMetal::Update() {
    // If some of the scene render related data changed, update the scene. Drawing is done after this.
    if (scene != nullptr) {
        scene->Update();
    }
    if (sceneMetal != nullptr) {
        sceneMetal->Update();
    }
    // If UI scene is set, update it too.
    if (uiScene != nullptr) {
        uiScene->Update();
    }
    if (uiSceneMetal != nullptr) {
        uiSceneMetal->Update();
    }
    // If the debug system's UI scene is set, update it too.
    if (debugUiScene != nullptr) {
        debugUiScene->Update();
    }
    if (debugUiSceneMetal != nullptr) {
        debugUiSceneMetal->Update();
    }
}

void ME::RendererMetal::End() {
    if (commandQueue != nullptr && fence != nullptr) {
        FlushCommandQueue();
    }

    delete sceneMetal;
    sceneMetal = nullptr;
    delete uiSceneMetal;
    uiSceneMetal = nullptr;
    delete debugUiSceneMetal;
    debugUiSceneMetal = nullptr;
    delete quad;
    quad = nullptr;

    NS::Object* objects[] = {depthTexture,  depthStateDisabled, depthStateDefault,
                             pso2DAtl,      pso2DInsAtl,        pso2DUIText,
                             pso2DUISprite, residencySet,       argumentTable,
                             compiler,      commandBuffer,      commandAllocator,
                             fence,         commandQueue,       device};
    for (NS::Object* object : objects) {
        if (object != nullptr) {
            object->release();
        }
    }
    depthTexture = nullptr;
    depthStateDisabled = nullptr;
    depthStateDefault = nullptr;
    pso2DAtl = nullptr;
    pso2DInsAtl = nullptr;
    pso2DUIText = nullptr;
    pso2DUISprite = nullptr;
    residencySet = nullptr;
    argumentTable = nullptr;
    compiler = nullptr;
    commandBuffer = nullptr;
    commandAllocator = nullptr;
    fence = nullptr;
    commandQueue = nullptr;
    device = nullptr;
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

    NS::Error* error = nullptr;
    MTL4::CompilerDescriptor* compilerDescriptor = MTL4::CompilerDescriptor::alloc()->init();
    compiler = device->newCompiler(compilerDescriptor, &error);
    compilerDescriptor->release();

    MTL4::ArgumentTableDescriptor* tableDescriptor = MTL4::ArgumentTableDescriptor::alloc()->init();
    tableDescriptor->setMaxBufferBindCount(MetalBindings::BufferCount);
    tableDescriptor->setMaxTextureBindCount(MetalBindings::TextureCount);
    argumentTable = device->newArgumentTable(tableDescriptor, &error);
    tableDescriptor->release();

    MTL::ResidencySetDescriptor* residencyDescriptor = MTL::ResidencySetDescriptor::alloc()->init();
    residencySet = device->newResidencySet(residencyDescriptor, &error);
    residencyDescriptor->release();

    if (compiler == nullptr || argumentTable == nullptr || residencySet == nullptr) {
        ME::LogError("RendererMetal: couldn't create the compiler, argument table or residency set");
        return false;
    }

    // Create PSOs and depth states.
    pso2DAtl = PSOMetal::CreatePSO2DAtlas(compiler, "sprite_atlas.metal");
    pso2DInsAtl = PSOMetal::CreatePSO2DInstancedAtlas(compiler, "sprite_instanced_atlas.metal");
    pso2DUIText = PSOMetal::CreatePSO2DUIText(compiler, "ui_text.metal");
    pso2DUISprite = PSOMetal::CreatePSO2DUISprite(compiler, "ui_sprite.metal");
    depthStateDefault = PSOMetal::CreateDepthStateDefault(device);
    depthStateDisabled = PSOMetal::CreateDepthStateDisabled(device);

    quad = new QuadMetal();
    quad->CreateBuffers(device);
    residencySet->addAllocation(quad->vertexBuffer);
    residencySet->addAllocation(quad->indexBuffer);

    const CGSize drawableSize = view->GetLayer()->drawableSize();
    EnsureDepthTexture(static_cast<uint32_t>(drawableSize.width), static_cast<uint32_t>(drawableSize.height));
    residencySet->commit();

    // Metal 4 makes nothing resident by itself: the renderer's set, and the layer's for its drawables.
    commandQueue->addResidencySet(residencySet);
    commandQueue->addResidencySet(view->GetLayer()->residencySet());
    view->GetLayer()->setDisplaySyncEnabled(vsyncEnabled);

    ME::Log("RendererMetal::InitMetal");
    return true;
}

void ME::RendererMetal::Draw() {
    if (commandQueue == nullptr || fence == nullptr || sceneMetal == nullptr || pso2DAtl == nullptr ||
        pso2DInsAtl == nullptr || pso2DUIText == nullptr || pso2DUISprite == nullptr) {
        return;
    }

    // Both are nil while the view has no drawable to give (e.g. the window is hidden); skip the frame then.
    MTL4::RenderPassDescriptor* renderPass = view->GetCurrentRenderPass();
    CA::MetalDrawable* drawable = view->GetCurrentDrawable();
    if (renderPass == nullptr || drawable == nullptr) {
        return;
    }
    const uint32_t width = static_cast<uint32_t>(drawable->texture()->width());
    const uint32_t height = static_cast<uint32_t>(drawable->texture()->height());
    EnsureDepthTexture(width, height);

    MTL::RenderPassColorAttachmentDescriptor* colorTarget = renderPass->colorAttachments()->object(0);
    colorTarget->setLoadAction(MTL::LoadActionClear);
    colorTarget->setClearColor(kClearColor);
    MTL::RenderPassDepthAttachmentDescriptor* depthTarget = renderPass->depthAttachment();
    depthTarget->setTexture(depthTexture);
    depthTarget->setLoadAction(MTL::LoadActionClear);
    depthTarget->setClearDepth(1.0);
    depthTarget->setStoreAction(MTL::StoreActionDontCare);

    // The previous frame was flushed, so its allocator memory is free to reuse.
    commandAllocator->reset();
    commandBuffer->beginCommandBuffer(commandAllocator);

    MTL4::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    encoder->setViewport(MTL::Viewport{0.0, 0.0, static_cast<double>(width), static_cast<double>(height), 0.0, 1.0});
    // Counter-clockwise front faces, back faces culled: the DX rasterizer state.
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);
    encoder->setArgumentTable(argumentTable, MTL::RenderStageVertex | MTL::RenderStageFragment);
    argumentTable->setAddress(quad->vertexBuffer->gpuAddress(), MetalBindings::VertexBuffer);

    ///////////////////////
    // Sprite Drawing.

    if (sceneMetal->spriteRenderers.count != 0) {
        encoder->setRenderPipelineState(pso2DAtl);
        encoder->setDepthStencilState(depthStateDefault);

        CBPerPass perPassData{};
        perPassData.viewMatrix = sceneMetal->spriteCamera->GetViewMatrix().GetDataForShader();
        perPassData.projectionMatrix = sceneMetal->spriteCamera->GetProjectionMatrix().GetDataForShader();
        perPassData.ambientLightData = sceneMetal->ambientLight->GetLightDataAmbient();
        perPassData.directionalLightData = sceneMetal->directionalLight->GetLightDataDirectional();
        sceneMetal->perPassCBs[0]->CopyData(&perPassData);
        argumentTable->setAddress(sceneMetal->perPassCBs[0]->GetGPUAddress(), MetalBindings::PerPassBuffer);

        for (uint32_t i = 0; i < sceneMetal->spriteRenderers.count; ++i) {
            const uint32_t textureIndex = sceneMetal->spriteRenderers[i].textureId;
            const uint32_t atlasPropsIndex = sceneMetal->spriteRenderers[i].textureAtlasPropsId;

            ME::TextureAtlasProperties atlasProps = sceneMetal->textureAtlasProperties[atlasPropsIndex];
            sceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);
            argumentTable->setAddress(sceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(),
                                      MetalBindings::AtlasBuffer);

            argumentTable->setTexture(sceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                      MetalBindings::Texture);

            CBPerSprite perSpriteData{};
            perSpriteData.modelMatrix = sceneMetal->spriteTransforms[i].GetModelMatrix().GetDataForShader();
            perSpriteData.color = sceneMetal->spriteRenderers[i].color;
            perSpriteData.atlasIndex = sceneMetal->spriteRenderers[i].atlasIndex;
            perSpriteData.flags = sceneMetal->spriteRenderers[i].flags;
            sceneMetal->perSpriteCBs[i]->CopyData(&perSpriteData);
            argumentTable->setAddress(sceneMetal->perSpriteCBs[i]->GetGPUAddress(), MetalBindings::PerObjectBuffer);

            encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                           quad->indexBuffer->gpuAddress(), quad->indicesSize);
        }
    }

    //////////////////////////////////
    // First Instanced Sprite Drawing (texture and atlas slot 1).

    if (sceneMetal->instancedSpriteRenderers0.count != 0) {
        DrawInstancedSpriteSet(encoder, 1, sceneMetal->spriteInstanceData0, sceneMetal->spriteInstanceBuffer0,
                               static_cast<uint32_t>(sceneMetal->instancedSpriteRenderers0.count));
    }

    //////////////////////////////////
    // Second Instanced Sprite Drawing (texture and atlas slot 2).

    if (sceneMetal->instancedSpriteRenderers1.count != 0) {
        DrawInstancedSpriteSet(encoder, 2, sceneMetal->spriteInstanceData1, sceneMetal->spriteInstanceBuffer1,
                               static_cast<uint32_t>(sceneMetal->instancedSpriteRenderers1.count));
    }
    // End Sprite Drawing.

    ///////////////////////////
    // UI Drawing (game's own UI, then the debug system on top of it).
    DrawUIScene(encoder, uiSceneMetal);

    DrawUIScene(encoder, debugUiSceneMetal);

    /////////////////////
    // Drawing End

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

void ME::RendererMetal::DrawInstancedSpriteSet(MTL4::RenderCommandEncoder* encoder, uint32_t textureIndex,
                                               const ME::SpriteRendererInstanceData* instanceData,
                                               ME::UploadBufferMetal* instanceBuffer, uint32_t instanceCount) {
    encoder->setRenderPipelineState(pso2DInsAtl);
    encoder->setDepthStencilState(depthStateDefault);

    // The texture and its atlas properties share the slot index, as in RendererDX.
    const uint32_t atlasPropsIndex = textureIndex;

    CBPerPass perPassData{};
    perPassData.viewMatrix = sceneMetal->spriteCamera->GetViewMatrix().GetDataForShader();
    perPassData.projectionMatrix = sceneMetal->spriteCamera->GetProjectionMatrix().GetDataForShader();
    perPassData.ambientLightData = sceneMetal->ambientLight->GetLightDataAmbient();
    perPassData.directionalLightData = sceneMetal->directionalLight->GetLightDataDirectional();
    sceneMetal->perPassCBs[0]->CopyData(&perPassData);
    argumentTable->setAddress(sceneMetal->perPassCBs[0]->GetGPUAddress(), MetalBindings::PerPassBuffer);

    ME::TextureAtlasProperties atlasProps = sceneMetal->textureAtlasProperties[atlasPropsIndex];
    sceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);
    argumentTable->setAddress(sceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(),
                              MetalBindings::AtlasBuffer);

    argumentTable->setTexture(sceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                              MetalBindings::Texture);

    instanceBuffer->CopyData(instanceData);
    argumentTable->setAddress(instanceBuffer->GetGPUAddress(), MetalBindings::InstanceBuffer);

    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                   quad->indexBuffer->gpuAddress(), quad->indicesSize, instanceCount);
}

void ME::RendererMetal::DrawUIScene(MTL4::RenderCommandEncoder* encoder, ME::SceneUIMetal* uiSceneMetal) {
    if (uiSceneMetal == nullptr) {
        return;
    }

    // UI is drawn over the sprites, in UI space (pixels from the top left) mapped by the window's size.
    encoder->setDepthStencilState(depthStateDisabled);
    const uint32_t screenDimension = ME::Utils::Pack16To32(window->width, window->height);

    ///////////////////////////
    // UI Sprite Drawing (atlas at texture slot 0).
    if (uiSceneMetal->uiSpriteInstanceData.count != 0) {
        encoder->setRenderPipelineState(pso2DUISprite);

        const uint32_t textureIndex = 0;
        const uint32_t atlasPropsIndex = 0;
        const uint32_t cbPerPassIndex = 0;  // UI Sprite CB is at index 0.

        CBPerPassUISprite perPassData{};
        perPassData.screenDimension = screenDimension;
        uiSceneMetal->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);
        argumentTable->setAddress(uiSceneMetal->perPassCBs[cbPerPassIndex]->GetGPUAddress(),
                                  MetalBindings::PerPassBuffer);

        ME::TextureAtlasProperties atlasProps = uiSceneMetal->textureAtlasProperties[atlasPropsIndex];
        uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);
        argumentTable->setAddress(uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(),
                                  MetalBindings::AtlasBuffer);

        argumentTable->setTexture(uiSceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                  MetalBindings::Texture);

        uiSceneMetal->uiSpriteInstanceBuffer->CopyData(uiSceneMetal->uiSpriteInstanceData.data);
        argumentTable->setAddress(uiSceneMetal->uiSpriteInstanceBuffer->GetGPUAddress(), MetalBindings::InstanceBuffer);

        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                       quad->indexBuffer->gpuAddress(), quad->indicesSize,
                                       uiSceneMetal->uiSpriteInstanceData.count);
    }

    ///////////////////////////
    // Text Drawing (font at texture slot 1).
    if (uiSceneMetal->textInstanceData.count != 0) {
        encoder->setRenderPipelineState(pso2DUIText);

        const uint32_t textureIndex = 1;
        const uint32_t atlasPropsIndex = 1;
        const uint32_t cbPerPassIndex = 1;  // UI Text CB is at index 1.

        CBPerPassUIText perPassData{};
        perPassData.screenDimension = screenDimension;
        uiSceneMetal->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);
        argumentTable->setAddress(uiSceneMetal->perPassCBs[cbPerPassIndex]->GetGPUAddress(),
                                  MetalBindings::PerPassBuffer);

        ME::TextureAtlasProperties atlasProps = uiSceneMetal->textureAtlasProperties[atlasPropsIndex];
        uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);
        argumentTable->setAddress(uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(),
                                  MetalBindings::AtlasBuffer);

        argumentTable->setTexture(uiSceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                  MetalBindings::Texture);

        uiSceneMetal->textInstanceBuffer->CopyData(uiSceneMetal->textInstanceData.data);
        argumentTable->setAddress(uiSceneMetal->textInstanceBuffer->GetGPUAddress(), MetalBindings::InstanceBuffer);

        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                       quad->indexBuffer->gpuAddress(), quad->indicesSize,
                                       uiSceneMetal->textInstanceData.count);
    }
}

void ME::RendererMetal::FlushCommandQueue() {
    ++currentFence;
    commandQueue->signalEvent(fence, currentFence);
    if (!fence->waitUntilSignaledValue(currentFence, kGpuWaitMilliseconds)) {
        ME::LogError("RendererMetal: the GPU didn't finish a frame in time");
    }
}

void ME::RendererMetal::EnsureDepthTexture(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }
    if (depthTexture != nullptr && depthTexture->width() == width && depthTexture->height() == height) {
        return;
    }
    // The GPU is idle here (every frame is flushed), so the old texture can go at once.
    if (depthTexture != nullptr) {
        residencySet->removeAllocation(depthTexture);
        depthTexture->release();
    }

    MTL::TextureDescriptor* descriptor =
        MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float, width, height, false);
    descriptor->setUsage(MTL::TextureUsageRenderTarget);
    descriptor->setStorageMode(MTL::StorageModePrivate);
    depthTexture = device->newTexture(descriptor);
    if (depthTexture != nullptr) {
        residencySet->addAllocation(depthTexture);
    }
    residencySet->commit();
}

#endif  // ME_METAL
