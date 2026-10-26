#ifdef ME_METAL

#include "renderer_metal.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "metal_view.h"
#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/light.h"
#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/scene/scene_metal.h"
#include "moksha/logging/logging.h"
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
    // The UI passes come with SceneUIMetal; until then the UI scene is only updated.
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
    // If some of the scene render related data changed, update the scene. Drawing is done after this.
    if (scene != nullptr) {
        scene->Update();
    }
    if (sceneMetal != nullptr) {
        sceneMetal->Update();
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

    delete sceneMetal;
    sceneMetal = nullptr;
    delete quad;
    quad = nullptr;

    NS::Object* objects[] = {
        depthTexture, depthStateDisabled, depthStateDefault, pso2DAtl, pso2DInsAtl,  residencySet, argumentTable,
        compiler,     commandBuffer,      commandAllocator,  fence,    commandQueue, device};
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
        pso2DInsAtl == nullptr) {
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
