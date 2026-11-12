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

// Longest wait for the GPU before giving up, per frame being waited for.
static constexpr uint64_t kGpuWaitMilliseconds = 1000;

void ME::RendererMetal::SetScene(ME::Scene* gameScene) {
    FlushCommandQueue();
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
    FlushCommandQueue();
    delete debugUiSceneMetal;
    debugUiSceneMetal = nullptr;
    debugUiScene = inDebugUiScene;
    if (residencySet == nullptr || inDebugUiScene == nullptr) {
        return;
    }
    debugUiSceneMetal = new ME::SceneUIMetal(device, residencySet, inDebugUiScene);
    residencySet->commit();
}

void ME::RendererMetal::OnPause() {
    FlushCommandQueue();
}

void ME::RendererMetal::SetVsyncEnabled(bool enabled) {
    vsyncEnabled = enabled;
    if (view != nullptr) {
        view->SetVsync(vsyncEnabled);
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
    FlushCommandQueue();

    delete sceneMetal;
    sceneMetal = nullptr;
    delete uiSceneMetal;
    uiSceneMetal = nullptr;
    delete debugUiSceneMetal;
    debugUiSceneMetal = nullptr;
    delete quad;
    quad = nullptr;

    for (MTL4::CommandAllocator*& allocator : commandAllocators) {
        if (allocator != nullptr) {
            allocator->release();
            allocator = nullptr;
        }
    }

    NS::Object* objects[] = {depthTexture,
                             depthStateDisabled,
                             depthStateTestOnly,
                             depthStateDefault,
                             pso2DAtl,
                             pso2DInsAtl,
                             pso2DInsAtlAdditive,
                             pso2DUIText,
                             pso2DUISprite,
                             residencySet,
                             argumentTable,
                             compiler,
                             commandBuffer,
                             fence,
                             commandQueue,
                             device};
    for (NS::Object* object : objects) {
        if (object != nullptr) {
            object->release();
        }
    }
    depthTexture = nullptr;
    depthStateDisabled = nullptr;
    depthStateDefault = nullptr;
    depthStateTestOnly = nullptr;
    pso2DAtl = nullptr;
    pso2DInsAtl = nullptr;
    pso2DInsAtlAdditive = nullptr;
    pso2DUIText = nullptr;
    pso2DUISprite = nullptr;
    residencySet = nullptr;
    argumentTable = nullptr;
    compiler = nullptr;
    commandBuffer = nullptr;
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
    commandBuffer = device->newCommandBuffer();
    fence = device->newSharedEvent();
    bool allocatorsCreated = true;
    for (MTL4::CommandAllocator*& allocator : commandAllocators) {
        allocator = device->newCommandAllocator();
        allocatorsCreated = allocatorsCreated && allocator != nullptr;
    }
    if (commandQueue == nullptr || !allocatorsCreated || commandBuffer == nullptr || fence == nullptr) {
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
    pso2DInsAtl = PSOMetal::CreatePSO2DInstancedAtlas(compiler, "sprite_instanced_atlas.metal", ME::SpriteBlend::Alpha);
    pso2DInsAtlAdditive =
        PSOMetal::CreatePSO2DInstancedAtlas(compiler, "sprite_instanced_atlas.metal", ME::SpriteBlend::Additive);
    pso2DUIText = PSOMetal::CreatePSO2DUIText(compiler, "ui_text.metal");
    pso2DUISprite = PSOMetal::CreatePSO2DUISprite(compiler, "ui_sprite.metal");
    depthStateDefault = PSOMetal::CreateDepthStateDefault(device);
    depthStateTestOnly = PSOMetal::CreateDepthStateTestOnly(device);
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
    view->SetVsync(vsyncEnabled);

    ME::Log("RendererMetal::InitMetal");
    return true;
}

void ME::RendererMetal::Draw() {
    if (commandQueue == nullptr || fence == nullptr || sceneMetal == nullptr || pso2DAtl == nullptr ||
        pso2DInsAtl == nullptr || pso2DInsAtlAdditive == nullptr || pso2DUIText == nullptr ||
        pso2DUISprite == nullptr || depthStateTestOnly == nullptr) {
        return;
    }

    // Usually instant: the last Draw already waited. If that wait timed out, the GPU may still be using this slot's
    // allocator and buffers, so skip the frame instead of overwriting them.
    if (!WaitForFence(frameFences[frameIndex], kGpuWaitMilliseconds)) {
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

    // The end of the last Draw waited for this slot's previous frame, so its allocator memory is free to reuse.
    commandAllocators[frameIndex]->reset();
    commandBuffer->beginCommandBuffer(commandAllocators[frameIndex]);

    MTL4::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    encoder->setViewport(MTL::Viewport{0.0, 0.0, static_cast<double>(width), static_cast<double>(height), 0.0, 1.0});
    // Counter-clockwise front faces, nothing culled: the DX 2D rasterizer state (a sprite mirrored by a negative
    // scale still draws).
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeNone);
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
        sceneMetal->perPassCBs[0]->CopyData(frameIndex, &perPassData);
        argumentTable->setAddress(sceneMetal->perPassCBs[0]->GetGPUAddress(frameIndex), MetalBindings::PerPassBuffer);

        for (uint32_t i = 0; i < sceneMetal->perSpriteCBCount; ++i) {
            const uint32_t textureIndex = sceneMetal->spriteRenderers[i].textureId;
            const uint32_t atlasPropsIndex = sceneMetal->spriteRenderers[i].textureAtlasPropsId;

            ME::TextureAtlasProperties atlasProps = sceneMetal->textureAtlasProperties[atlasPropsIndex];
            sceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(frameIndex, &atlasProps);
            argumentTable->setAddress(sceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(frameIndex),
                                      MetalBindings::AtlasBuffer);

            argumentTable->setTexture(sceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                      MetalBindings::Texture);

            CBPerSprite perSpriteData{};
            perSpriteData.modelMatrix = sceneMetal->spriteTransforms[i].GetModelMatrix().GetDataForShader();
            perSpriteData.color = sceneMetal->spriteRenderers[i].color;
            perSpriteData.atlasIndex = sceneMetal->spriteRenderers[i].atlasIndex;
            perSpriteData.flags = sceneMetal->spriteRenderers[i].flags;
            sceneMetal->perSpriteCBs[i]->CopyData(frameIndex, &perSpriteData);
            argumentTable->setAddress(sceneMetal->perSpriteCBs[i]->GetGPUAddress(frameIndex),
                                      MetalBindings::PerObjectBuffer);

            encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                           quad->indexBuffer->gpuAddress(), quad->indicesSize);
        }
    }

    //////////////////////////////////
    // First Instanced Sprite Drawing (texture and atlas slot 1).

    const uint32_t drawCount0 = scene->GetInstancedDrawCount0();
    if (drawCount0 != 0 && sceneMetal->spriteInstanceBuffer0 != nullptr) {
        DrawInstancedSpriteSet(encoder, 1, sceneMetal->spriteInstanceData0, sceneMetal->spriteInstanceBuffer0,
                               drawCount0, scene->instancedSpriteBlend0);
    }

    //////////////////////////////////
    // Second Instanced Sprite Drawing (texture and atlas slot 2).

    const uint32_t drawCount1 = scene->GetInstancedDrawCount1();
    if (drawCount1 != 0 && sceneMetal->spriteInstanceBuffer1 != nullptr) {
        DrawInstancedSpriteSet(encoder, 2, sceneMetal->spriteInstanceData1, sceneMetal->spriteInstanceBuffer1,
                               drawCount1, scene->instancedSpriteBlend1);
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

    // Mark when this slot is free, then wait here for the next slot, so the next frame's input isn't read early.
    ++currentFence;
    commandQueue->signalEvent(fence, currentFence);
    frameFences[frameIndex] = currentFence;
    frameIndex = (frameIndex + 1) % static_cast<uint32_t>(Constants::FramesInFlight);
    WaitForFence(frameFences[frameIndex], kGpuWaitMilliseconds);
}

void ME::RendererMetal::DrawInstancedSpriteSet(MTL4::RenderCommandEncoder* encoder, uint32_t textureIndex,
                                               const ME::SpriteRendererInstanceData* instanceData,
                                               ME::UploadBufferMetal* instanceBuffer, uint32_t instanceCount,
                                               ME::SpriteBlend blend) {
    const bool bAdditive = blend == ME::SpriteBlend::Additive;
    encoder->setRenderPipelineState(bAdditive ? pso2DInsAtlAdditive : pso2DInsAtl);
    encoder->setDepthStencilState(bAdditive ? depthStateTestOnly : depthStateDefault);

    // The texture and its atlas properties share the slot index, as in RendererDX.
    const uint32_t atlasPropsIndex = textureIndex;

    CBPerPass perPassData{};
    perPassData.viewMatrix = sceneMetal->spriteCamera->GetViewMatrix().GetDataForShader();
    perPassData.projectionMatrix = sceneMetal->spriteCamera->GetProjectionMatrix().GetDataForShader();
    perPassData.ambientLightData = sceneMetal->ambientLight->GetLightDataAmbient();
    perPassData.directionalLightData = sceneMetal->directionalLight->GetLightDataDirectional();
    sceneMetal->perPassCBs[0]->CopyData(frameIndex, &perPassData);
    argumentTable->setAddress(sceneMetal->perPassCBs[0]->GetGPUAddress(frameIndex), MetalBindings::PerPassBuffer);

    ME::TextureAtlasProperties atlasProps = sceneMetal->textureAtlasProperties[atlasPropsIndex];
    sceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(frameIndex, &atlasProps);
    argumentTable->setAddress(sceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(frameIndex),
                              MetalBindings::AtlasBuffer);

    argumentTable->setTexture(sceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                              MetalBindings::Texture);

    instanceBuffer->CopyElements(frameIndex, instanceData, instanceCount);
    argumentTable->setAddress(instanceBuffer->GetGPUAddress(frameIndex), MetalBindings::InstanceBuffer);

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
        uiSceneMetal->perPassCBs[cbPerPassIndex]->CopyData(frameIndex, &perPassData);
        argumentTable->setAddress(uiSceneMetal->perPassCBs[cbPerPassIndex]->GetGPUAddress(frameIndex),
                                  MetalBindings::PerPassBuffer);

        ME::TextureAtlasProperties atlasProps = uiSceneMetal->textureAtlasProperties[atlasPropsIndex];
        uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(frameIndex, &atlasProps);
        argumentTable->setAddress(uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(frameIndex),
                                  MetalBindings::AtlasBuffer);

        argumentTable->setTexture(uiSceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                  MetalBindings::Texture);

        uiSceneMetal->uiSpriteInstanceBuffer->CopyElements(
            frameIndex, uiSceneMetal->uiSpriteInstanceData.data,
            static_cast<uint32_t>(uiSceneMetal->uiSpriteInstanceData.count));
        argumentTable->setAddress(uiSceneMetal->uiSpriteInstanceBuffer->GetGPUAddress(frameIndex),
                                  MetalBindings::InstanceBuffer);

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
        uiSceneMetal->perPassCBs[cbPerPassIndex]->CopyData(frameIndex, &perPassData);
        argumentTable->setAddress(uiSceneMetal->perPassCBs[cbPerPassIndex]->GetGPUAddress(frameIndex),
                                  MetalBindings::PerPassBuffer);

        ME::TextureAtlasProperties atlasProps = uiSceneMetal->textureAtlasProperties[atlasPropsIndex];
        uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->CopyData(frameIndex, &atlasProps);
        argumentTable->setAddress(uiSceneMetal->textureAtlasCBs[atlasPropsIndex]->GetGPUAddress(frameIndex),
                                  MetalBindings::AtlasBuffer);

        argumentTable->setTexture(uiSceneMetal->spriteTextures[textureIndex]->GetTexture()->gpuResourceID(),
                                  MetalBindings::Texture);

        uiSceneMetal->textInstanceBuffer->CopyElements(frameIndex, uiSceneMetal->textInstanceData.data,
                                                       static_cast<uint32_t>(uiSceneMetal->textInstanceData.count));
        argumentTable->setAddress(uiSceneMetal->textInstanceBuffer->GetGPUAddress(frameIndex),
                                  MetalBindings::InstanceBuffer);

        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, quad->indexCount, MTL::IndexTypeUInt16,
                                       quad->indexBuffer->gpuAddress(), quad->indicesSize,
                                       uiSceneMetal->textInstanceData.count);
    }
}

void ME::RendererMetal::FlushCommandQueue() {
    if (commandQueue == nullptr || fence == nullptr) {
        return;
    }
    ++currentFence;
    commandQueue->signalEvent(fence, currentFence);
    WaitForFence(currentFence, kGpuWaitMilliseconds * Constants::FramesInFlight);
}

bool ME::RendererMetal::WaitForFence(uint64_t value, uint64_t timeoutMilliseconds) {
    if (!fence->waitUntilSignaledValue(value, timeoutMilliseconds)) {
        ME::LogError("RendererMetal: the GPU didn't finish a frame in time");
        return false;
    }
    return true;
}

void ME::RendererMetal::EnsureDepthTexture(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }
    if (depthTexture != nullptr && depthTexture->width() == width && depthTexture->height() == height) {
        return;
    }
    // Frames still in flight may be using the old texture, and Metal 4 doesn't keep it alive for them.
    if (depthTexture != nullptr) {
        FlushCommandQueue();
        depthTexture->release();
    }

    // Memoryless: depth lives in tile memory for one pass and is never stored, so overlapping frames don't conflict.
    MTL::TextureDescriptor* descriptor =
        MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float, width, height, false);
    descriptor->setUsage(MTL::TextureUsageRenderTarget);
    descriptor->setStorageMode(MTL::StorageModeMemoryless);
    depthTexture = device->newTexture(descriptor);
}

#endif  // ME_METAL
