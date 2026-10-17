#ifdef ME_DX12

#include "scene_dx.h"

#include "moksha/client/rendering/directx/pso_dx.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneDX::SceneDX(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ME::DescHeapManagerDX* descHeapManager,
                     ME::Scene* gameScene) {
    this->device = device;
    this->cmdList = cmdList;
    this->descHeapManager = descHeapManager;

    scene = gameScene;
    this->ambientLight = scene->ambientLight;
    this->directionalLight = scene->directionalLight;
    this->camera = scene->camera;
    this->spriteCamera = scene->spriteCamera;

    meshes = new ME::MeshDX*[Constants::MaxMeshCount];
    quads = new ME::QuadDX*[Constants::MaxQuadCount];
    textures = new ME::TextureDX*[Constants::MaxTextureCount];
    spriteTextures = new ME::TextureDX*[Constants::MaxSpriteTextureCount];
    // textureSamplerStates = new MTL::SamplerState*[Constants::MaxSamplerCount];
    perPassCBs = new ME::UploadBufferDX*[Constants::MaxConstantBuffersCount];
    perPassCBHeapIndices = new uint32_t[Constants::MaxConstantBuffersCount];
    perSpriteCBs = new ME::UploadBufferDX*[Constants::MaxConstantBuffersCount];
    perSpriteCBHeapIndices = new uint32_t[Constants::MaxConstantBuffersCount];
    textureAtlasCBs = new ME::UploadBufferDX*[Constants::MaxConstantBuffersCount];
    textureAtlasCBHeapIndices = new uint32_t[Constants::MaxConstantBuffersCount];
    transforms = scene->transforms;
    spriteTransforms = scene->spriteTransforms;

    instancedSpriteTransforms0 = scene->instancedSpriteTransforms0;

    instancedSpriteTransforms1 = scene->instancedSpriteTransforms1;

    meshRenderers = scene->meshRenderers;
    spriteRenderers = scene->spriteRenderers;

    instancedSpriteRenderers0 = scene->instancedSpriteRenderers0;
    spriteInstanceData0 = scene->spriteInstanceData0;

    instancedSpriteRenderers1 = scene->instancedSpriteRenderers1;
    spriteInstanceData1 = scene->spriteInstanceData1;

    textureAtlasProperties = scene->textureAtlasProperties;

    MakeMeshes();
    MakeQuads();
    MakeConstantBuffers();
    MakeTextures();
    MakeSpriteTextures();
    MakeShaders();
    MakeTextureSamplers();

    MakeSpriteInstanceBuffer();
}

ME::SceneDX::~SceneDX() {
    delete ambientLight;
    delete directionalLight;
    delete camera;
    delete spriteCamera;

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
        // textureSamplerStates[i]->release();
    }
    // delete[] textureSamplerStates;

    for (uint32_t i = 0; i < perPassCBCount; i++) {
        delete perPassCBs[i];
    }
    delete[] perPassCBs;
    delete[] perPassCBHeapIndices;

    for (uint32_t i = 0; i < perSpriteCBCount; i++) {
        delete perSpriteCBs[i];
    }
    delete[] perSpriteCBs;
    delete[] perSpriteCBHeapIndices;

    for (uint32_t i = 0; i < textureAtlasCBCount; i++) {
        delete textureAtlasCBs[i];
    }
    delete[] textureAtlasCBs;
    delete[] textureAtlasCBHeapIndices;
}

void ME::SceneDX::Init() {}

void ME::SceneDX::PostInitCleanup() {
    for (uint8_t i = 0; i < scene->meshPaths.count; i++) {
        meshes[i]->ReleaseUploadBuffers();
    }

    for (uint8_t i = 0; i < scene->quadPaths.count; i++) {
        quads[i]->ReleaseUploadBuffers();
    }

    for (uint8_t i = 0; i < scene->texturePaths.count; i++) {
        textures[i]->ReleaseUploadBuffers();
    }

    for (uint8_t i = 0; i < scene->spriteTexturePaths.count; i++) {
        spriteTextures[i]->ReleaseUploadBuffers();
    }
}

void ME::SceneDX::Update() {}

void ME::SceneDX::End() {}

void ME::SceneDX::MakeMeshes() {
    for (uint8_t i = 0; i < scene->meshPaths.count; i++) {
        meshes[i] = new ME::MeshDX(scene->meshPaths[i], device, cmdList);
        meshes[i]->CreateBuffers(device, cmdList);
    }
}

void ME::SceneDX::MakeQuads() {
    for (uint8_t i = 0; i < scene->quadPaths.count; i++) {
        quads[i] = new ME::QuadDX(scene->quadPaths[i], device, cmdList);
        quads[i]->CreateBuffers(device, cmdList);
    }
}

void ME::SceneDX::MakeTextures() {
    for (uint8_t i = 0; i < scene->texturePaths.count; i++) {
        textures[i] = new ME::TextureDX(scene->texturePaths[i], device, cmdList);
        textures[i]->CreateBuffers(device, cmdList);
        textures[i]->descHeapIndex = descHeapManager->CreateSRVTexture(textures[i]->GetTextureBuffer());
    }
}

void ME::SceneDX::MakeSpriteTextures() {
    for (uint8_t i = 0; i < scene->spriteTexturePaths.count; i++) {
        spriteTextures[i] = new ME::TextureDX(scene->spriteTexturePaths[i], device, cmdList);
        spriteTextures[i]->CreateBuffers(device, cmdList);
        spriteTextures[i]->descHeapIndex = descHeapManager->CreateSRVTexture(spriteTextures[i]->GetTextureBuffer());
    }
}

void ME::SceneDX::MakeShaders() {}

void ME::SceneDX::MakeTextureSamplers() {}

void ME::SceneDX::MakeConstantBuffers() {
    // Creating per-pass constant buffers.
    perPassCBCount = 1;

    // Sprite Rendering.
    perPassCBs[0] = new ME::UploadBufferDX(device, true, 1, sizeof(ME::CBPerPass));
    perPassCBHeapIndices[0] = descHeapManager->CreateCBV(perPassCBs[0]->GetResource(), perPassCBs[0]->GetElementSize());

    // Creating per-sprite constant buffers.
    perSpriteCBCount = spriteRenderers.count;
    for (uint32_t i = 0; i < perSpriteCBCount; ++i) {
        perSpriteCBs[i] = new ME::UploadBufferDX(device, true, 1, sizeof(ME::CBPerSprite));
        perSpriteCBHeapIndices[i] =
            descHeapManager->CreateCBV(perSpriteCBs[i]->GetResource(), perSpriteCBs[i]->GetElementSize());
    }

    // Creating texture atlas constant buffers.
    textureAtlasCBCount = scene->textureAtlasProperties.count;
    for (uint32_t i = 0; i < textureAtlasCBCount; ++i) {
        textureAtlasCBs[i] = new ME::UploadBufferDX(device, true, 1, sizeof(ME::TextureAtlasProperties));
        textureAtlasCBHeapIndices[i] =
            descHeapManager->CreateCBV(textureAtlasCBs[i]->GetResource(), textureAtlasCBs[i]->GetElementSize());
    }
}

void ME::SceneDX::MakeSpriteInstanceBuffer() {
    // First instance buffer.
    if (instancedSpriteRenderers0.count == 0) {
        return;
    }

    spriteInstanceBuffer0 =
        new ME::UploadBufferDX(device, false, instancedSpriteRenderers0.count, sizeof(ME::SpriteRendererInstanceData));
    spriteInstanceBufferHeapIndex0 = descHeapManager->CreateSRVInstanceData(
        spriteInstanceBuffer0->GetResource(), sizeof(ME::SpriteRendererInstanceData), instancedSpriteRenderers0.count);

    // Second instance buffer.
    if (instancedSpriteRenderers1.count == 0) {
        return;
    }

    spriteInstanceBuffer1 =
        new ME::UploadBufferDX(device, false, instancedSpriteRenderers1.count, sizeof(ME::SpriteRendererInstanceData));
    spriteInstanceBufferHeapIndex1 = descHeapManager->CreateSRVInstanceData(
        spriteInstanceBuffer1->GetResource(), sizeof(ME::SpriteRendererInstanceData), instancedSpriteRenderers1.count);
}

#endif  // ME_DX12
