#ifdef ME_METAL

/**
 * It takes a cross platform scene and converts it to a Metal scene.
 * This will create metal sepcific objects like buffers, textures, shaders, etc.
 */

#pragma once

#include <simd/simd.h>

#include <Metal/Metal.hpp>

#include "moksha/client/rendering/metal/mesh_metal.h"
#include "moksha/client/rendering/metal/quad_metal.h"
#include "moksha/client/rendering/metal/texture_metal.h"
#include "scene.h"

namespace ME {

class SceneMetal {
   public:
    SceneMetal() = delete;
    SceneMetal(MTL::Device* device, MTL::CommandQueue* cmdQueue, ME::Scene* gameScene);
    ~SceneMetal();

    // Init game
    void Init();

    // Update scene. Mostly used to update the transforms and instance count. Actual scene change is done in the
    // ME::Scene class.
    void Update();

    void End();

    ME::Light* ambientLight;
    ME::Light* directionalLight;
    ME::Camera* camera;
    ME::Camera* spriteCamera;

    ME::MeshMetal** meshes;
    ME::QuadMetal** quads;
    ME::TextureMetal** textures;
    ME::TextureMetal** spriteTextures;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;
    MTL::SamplerState** textureSamplerStates;
    ME::Transform** transforms;
    ME::MeshRenderer** meshRenderers;
    ME::Transform** spriteTransforms;
    ME::SpriteRenderer** spriteRenderers;
    ME::Transform** instancedSpriteTransforms;
    ME::SpriteRenderer** instancedSpriteRenderers;
    ME::Transform** textTransforms;
    ME::TextRenderer** textRenderers;

    uint16_t transformCount;
    uint16_t meshRendererCount;
    uint16_t spriteTransformCount;
    uint16_t spriteRendererCount;
    uint32_t instancedSpriteTransformCount;
    uint32_t instancedSpriteRendererCount;
    uint32_t textTransformsCount;
    uint32_t textRendererCount;
    uint32_t textInstanceDataCount;

    ME::SpriteRendererInstanceData** spriteInstanceData;
    MTL::Buffer* spriteInstanceBuffer;

    ME::TextRendererInstanceData** textInstanceData;
    MTL::Buffer* textInstanceBuffer;

   private:
    MTL::Device* device;
    MTL::CommandQueue* cmdQueue;
    ME::Scene* scene;

    void MakeMeshes();
    void MakeQuads();
    void MakeTextures();
    void MakeSpriteTextures();
    void MakeShaders();
    void MakeTextureSamplers();
    void MakeSpriteInstanceBuffer();
    void MakeTextInstanceBuffer();
};

}  // namespace ME

#endif
