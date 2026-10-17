#ifdef ME_MAC

#include "scene_metal.h"

#include "moksha/shared/misc/engine_limits.h"

ME::SceneMetal::SceneMetal(MTL::Device* device, MTL::CommandQueue* cmdQueue, ME::Scene* gameScene) {
    this->device = device;
    this->cmdQueue = cmdQueue;

    scene = gameScene;
    this->ambientLight = scene->ambientLight;
    this->directionalLight = scene->directionalLight;
    this->camera = scene->camera;
    this->spriteCamera = scene->spriteCamera;

    meshes = new ME::MeshMetal*[Constants::MaxMeshCount];
    quads = new ME::QuadMetal*[Constants::MaxQuadCount];
    textures = new ME::TextureMetal*[Constants::MaxTextureCount];
    spriteTextures = new ME::TextureMetal*[Constants::MaxSpriteTextureCount];
    textureSamplerStates = new MTL::SamplerState*[Constants::MaxSamplerCount];

    transforms = scene->transforms;
    transformCount = scene->transformCount;
    spriteTransforms = scene->spriteTransforms;
    spriteTransformCount = scene->spriteTransformCount;
    instancedSpriteTransforms = scene->instancedSpriteTransforms;
    instancedSpriteTransformCount = scene->instancedSpriteTransformCount;
    textTransforms = scene->textTransforms;
    textTransformsCount = scene->textTransformsCount;

    meshRenderers = scene->meshRenderers;
    meshRendererCount = scene->meshRendererCount;
    spriteRenderers = scene->spriteRenderers;
    spriteRendererCount = scene->spriteRendererCount;
    instancedSpriteRenderers = scene->instancedSpriteRenderers;
    instancedSpriteRendererCount = scene->instancedSpriteRendererCount;
    spriteInstanceData = scene->spriteInstanceData;
    textureAtlasProperties = scene->textureAtlasProperties;
    textRenderers = scene->textRenderers;
    textRendererCount = scene->textRendererCount;
    textInstanceData = scene->textInstanceData;
    textInstanceDataCount = scene->textInstanceDataCount;

    MakeMeshes();
    MakeQuads();
    MakeTextures();
    MakeSpriteTextures();
    MakeShaders();
    MakeTextureSamplers();

    MakeSpriteInstanceBuffer();
    MakeTextInstanceBuffer();
}

ME::SceneMetal::~SceneMetal() {
    delete ambientLight;
    delete directionalLight;
    delete camera;

    for (uint8_t i = 0; i < scene->meshPaths.count; i++) {
        delete meshes[i];
    }
    delete[] meshes;

    for (uint8_t i = 0; i < scene->quadPaths.count; i++) {
        delete quads[i];
    }
    delete[] quads;

    for (uint8_t i = 0; i < scene->texturePaths.count; i++) {
        delete textures[i];
    }
    delete[] textures;

    for (uint8_t i = 0; i < scene->spriteTexturePaths.count; i++) {
        delete spriteTextures[i];
    }
    delete[] spriteTextures;

    for (uint8_t i = 0; i < scene->textureSamplers.count; i++) {
        textureSamplerStates[i]->release();
    }
    delete[] textureSamplerStates;
}

void ME::SceneMetal::MakeMeshes() {
    for (uint8_t i = 0; i < scene->meshPaths.count; i++) {
        meshes[i] = new ME::MeshMetal(scene->meshPaths[i], device, cmdQueue);
    }
}

void ME::SceneMetal::MakeQuads() {
    for (uint8_t i = 0; i < scene->quadPaths.count; i++) {
        quads[i] = new ME::QuadMetal(scene->quadPaths[i], device, cmdQueue);
    }
}

void ME::SceneMetal::MakeTextures() {
    for (uint8_t i = 0; i < scene->texturePaths.count; i++) {
        textures[i] = new ME::TextureMetal(scene->texturePaths[i], device, true, cmdQueue);
    }
}

void ME::SceneMetal::MakeSpriteTextures() {
    for (uint8_t i = 0; i < scene->spriteTexturePaths.count; i++) {
        spriteTextures[i] = new ME::TextureMetal(scene->spriteTexturePaths[i], device, false, cmdQueue);
    }
}

void ME::SceneMetal::MakeShaders() {}

void ME::SceneMetal::MakeTextureSamplers() {
    for (uint8_t i = 0; i < scene->textureSamplers.count; i++) {
        if (scene->textureSamplers[i].filter == ME::TextureFilter::Nearest) {
            textureSamplerStates[i] = ME::TextureMetal::GetSamplerStateNearest(device);
        } else {
            textureSamplerStates[i] = ME::TextureMetal::GetSamplerStateLinear(device);
        }
    }
}

void ME::SceneMetal::MakeSpriteInstanceBuffer() {
    spriteInstanceBuffer = device->newBuffer(sizeof(ME::SpriteRendererInstanceData) * instancedSpriteRendererCount,
                                             MTL::ResourceStorageModeManaged);
}

void ME::SceneMetal::MakeTextInstanceBuffer() {
    textInstanceBuffer = device->newBuffer(sizeof(ME::TextRendererInstanceData) * textInstanceDataCount,
                                           MTL::ResourceStorageModeManaged);
}

void ME::SceneMetal::Update() {}

#endif
