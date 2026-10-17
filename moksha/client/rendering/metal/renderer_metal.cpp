#ifdef ME_METAL

#include "renderer_metal.h"

#include <cmath>

#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/color.h"
#include "moksha/client/rendering/shared/mesh.h"
#include "moksha/client/rendering/shared/mesh_parser_obj.h"
#include "moksha/client/rendering/shared/texture.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/math/vec16.h"
#include "moksha/shared/math/vec3.h"
#include "shader/shader_mac.h"
#include "texture_metal.h"

void ME::RendererMetal::Init() {
    ME::Log("RendererMetal::Init");
}

void ME::RendererMetal::SetScene(ME::Scene* gameScene) {
    // Delete the old scene if it exists.
    if (scene != nullptr) {
        delete scene;
    }
    // Create a new scene with the provided game scene.
    scene = new ME::SceneMetal(device, commandQueue, gameScene);
}

void ME::RendererMetal::Update() {
    if (scene == nullptr) {
        return;
    }
    scene->Update();
}

void ME::RendererMetal::End() {
    commandQueue->release();
    device->release();
    view->release();
    delete scene;
}

void ME::RendererMetal::InitMTL(MTL::Device* inDevice, MTK::View* inView) {
    device = inDevice->retain();
    commandQueue = device->newCommandQueue();
    view = inView;
}

void ME::RendererMetal::SetVsyncEnabled(bool enabled) {
    // TODO: not actually wired to presentation yet - real vsync control on Metal is
    // CAMetalLayer::displaySyncEnabled, which isn't exposed by the vendored metal-cpp headers
    // in moksha/client/third_party/metal (vendored, treated as opaque). MTKView is
    // vsync-paced by default via its display-linked draw callback either way. Wiring this
    // properly needs either extending that vendored bridge or reaching the layer from the
    // Objective-C side (metal_view.mm). Stored for now so the setting round-trips correctly
    // once that's done.
    vsyncEnabled = enabled;
}

void ME::RendererMetal::Draw() {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer* cmd = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* rpd = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* enc = cmd->renderCommandEncoder(rpd);

    // // Draw   3D Items.
    // ME::LightDataAmbient lightDataAmbient = scene->ambientLight->GetLightDataAmbient();
    // ME::LightDataDirectional lightDataDirectional = scene->directionalLight->GetLightDataDirectional();

    // Vec16 viewMatVec = scene->camera->GetViewMatrix().GetData();
    // Vec16 projectionMatVec = scene->camera->GetProjectionMatrix().GetData();

    // enc->setRenderPipelineState(ME::RenderPipelineStateMetal::GetNewPSO(device));
    // enc->setDepthStencilState(ME::DepthStencilStateMetal::GetNewDepthStencilState(device));

    // enc->setVertexBytes(&viewMatVec, sizeof(ME::Vec16), 2);
    // enc->setVertexBytes(&projectionMatVec, sizeof(ME::Vec16), 3);

    // enc->setFragmentBytes(&lightDataAmbient, sizeof(ME::LightDataAmbient), 1);
    // enc->setFragmentBytes(&lightDataDirectional, sizeof(ME::LightDataDirectional), 2);

    // enc->setFragmentSamplerState(scene->textureSamplerStates[0], 0);

    // enc->setCullMode(MTL::CullModeBack);
    // enc->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);

    // for (uint16_t i = 0; i < scene->meshRendererCount; ++i) {
    //     ME::MeshMetal* mesh = scene->meshes[scene->meshRenderers[i]->meshId];
    //     ME::TextureMetal* texture = scene->textures[scene->meshRenderers[i]->textureId];

    //     enc->setVertexBuffer(mesh->vertexBuffer, 0, 0);
    //     Vec16 modelMatVec = scene->transforms[i]->GetModelMatrix().GetData();
    //     enc->setVertexBytes(&modelMatVec, sizeof(ME::Vec16), 1);
    //     enc->setVertexBytes(&scene->meshRenderers[i]->color, sizeof(ME::Color), 4);
    //     enc->setFragmentTexture(texture->GetTextureMetal(), 0);

    //     enc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, mesh->indexCount,
    //                                MTL::IndexType::IndexTypeUInt32, mesh->indexBuffer, 0, 1);
    // }

    // // Draw   2D Items.
    // enc->setRenderPipelineState(ME::RenderPipelineStateMetal::GetNewPSO2D(device));
    // enc->setDepthStencilState(ME::DepthStencilStateMetal::GetNewDepthStencilState2D(device));

    // enc->setVertexBuffer(scene->quads[0]->vertexBuffer, 0, 0);
    // Vec16 spriteViewMatVec = scene->spriteCamera->GetViewMatrix().GetData();
    // enc->setVertexBytes(&spriteViewMatVec, sizeof(ME::Vec16), 2);
    // Vec16 spriteProjectionMatVec = scene->spriteCamera->GetProjectionMatrix().GetData();
    // enc->setVertexBytes(&spriteProjectionMatVec, sizeof(ME::Vec16), 3);
    // ME::TextureMetal* texture = scene->spriteTextures[scene->spriteRenderers[0]->textureId];
    // enc->setFragmentTexture(texture->GetTextureMetal(), 0);
    // enc->setFragmentSamplerState(scene->textureSamplerStates[0], 0);
    // ME::QuadMetal* quad = scene->quads[scene->instancedSpriteRenderers[0]->quadId];

    // for (uint16_t i = 0; i < scene->spriteRendererCount; ++i) {
    //     ME::Color color = scene->spriteRenderers[i]->color;
    //     enc->setVertexBytes(&color, sizeof(ME::Color), 4);
    //     Vec16 modelMatVec = scene->spriteTransforms[i]->GetModelMatrix().GetData();
    //     enc->setVertexBytes(&modelMatVec, sizeof(ME::Vec16), 1);

    //     enc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, quad->indexCount,
    //                                MTL::IndexType::IndexTypeUInt16, quad->indexBuffer, 0, 1);
    // }

    // Draw Instanced Sprites.
    const char* insShaderPath = "shaders/metal/sprite_instanced.metal";
    MTL::RenderPipelineState* pso = ME::RenderPipelineStateMetal::GetNewPSO2D(device, insShaderPath);
    MTL::DepthStencilState* dss = ME::DepthStencilStateMetal::GetNewDepthStencilState2D(device);
    enc->setRenderPipelineState(pso);
    enc->setDepthStencilState(dss);

    ME::QuadMetal* quad = scene->quads[scene->instancedSpriteRenderers[0]->quadId];
    enc->setVertexBuffer(scene->quads[0]->vertexBuffer, 0, 0);
    Vec16 spriteViewMatVec2 = scene->spriteCamera->GetViewMatrix().GetData();
    enc->setVertexBytes(&spriteViewMatVec2, sizeof(ME::Vec16), 2);
    Vec16 spriteProjectionMatVec2 = scene->spriteCamera->GetProjectionMatrix().GetData();
    enc->setVertexBytes(&spriteProjectionMatVec2, sizeof(ME::Vec16), 3);
    ME::TextureMetal* texture2 = scene->spriteTextures[0];
    enc->setFragmentTexture(texture2->GetTextureMetal(), 0);
    enc->setFragmentSamplerState(scene->textureSamplerStates[0], 0);

    ME::TextureAtlasProperties atlasProps = scene->textureAtlasProperties[0];
    enc->setFragmentBytes(&atlasProps, sizeof(ME::TextureAtlasProperties), 1);

    // TODO: Use array of data for sprite instance data, instead of array of pointers.
    for (uint32_t i = 0; i < scene->instancedSpriteTransformCount; ++i) {
        ((ME::SpriteRendererInstanceData*)scene->spriteInstanceBuffer->contents())[i] = *(scene->spriteInstanceData[i]);
    }

    scene->spriteInstanceBuffer->didModifyRange(
        NS::Range(0, sizeof(ME::SpriteRendererInstanceData) * scene->instancedSpriteTransformCount));
    enc->setVertexBuffer(scene->spriteInstanceBuffer, 0, 1);

    enc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, quad->indexCount,
                               MTL::IndexType::IndexTypeUInt16, quad->indexBuffer, 0,
                               scene->instancedSpriteTransformCount);

    // Draw Text Items.
    ME::TextureMetal* textureText = scene->spriteTextures[1];
    enc->setFragmentTexture(textureText->GetTextureMetal(), 0);
    enc->setFragmentSamplerState(scene->textureSamplerStates[0], 0);

    ME::TextureAtlasProperties atlasProps2 = scene->textureAtlasProperties[1];
    enc->setFragmentBytes(&atlasProps2, sizeof(ME::TextureAtlasProperties), 1);

    for (uint32_t i = 0; i < scene->textInstanceDataCount; ++i) {
        ((ME::TextRendererInstanceData*)scene->textInstanceBuffer->contents())[i] = *(scene->textInstanceData[i]);
    }

    scene->textInstanceBuffer->didModifyRange(
        NS::Range(0, sizeof(ME::TextRendererInstanceData) * scene->textInstanceDataCount));
    enc->setVertexBuffer(scene->textInstanceBuffer, 0, 1);

    enc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, quad->indexCount,
                               MTL::IndexType::IndexTypeUInt16, quad->indexBuffer, 0, scene->textInstanceDataCount);

    // End of drawing.
    enc->endEncoding();
    cmd->presentDrawable(view->currentDrawable());
    cmd->commit();

    // Cleanup.
    pso->release();
    dss->release();
    pool->release();
}

#endif  // ME_METAL
