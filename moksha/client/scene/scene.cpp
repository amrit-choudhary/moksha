#include "scene.h"

#include <cmath>
#include <cstring>

#include "moksha/client/debug/debug_system.h"
#include "moksha/shared/misc/engine_limits.h"
#include "moksha/shared/random/random_engine.h"
#include "moksha/shared/random/stb_perlin.h"

ME::Scene::Scene() {}

void ME::Scene::Init() {
    CreateResources();
    BuildLights();
    BuildCamera();
    BuildTransforms();
    BuildMeshRenderers();
    BuildSpriteTransforms();
    BuildSpriteRenderers();
    BuildInstancedSpriteTransforms();
    BuildInstancedSpriteRenderers();

    ME::DebugSystem::ScreenPrintSlot(2, GetDisplayName());
}

ME::Scene::~Scene() {
    delete ambientLight;
    delete directionalLight;
    delete camera;
    delete spriteCamera;

    delete[] meshPaths.data;
    delete[] quadPaths.data;
    delete[] texturePaths.data;
    delete[] spriteTexturePaths.data;
    delete[] textureAtlasProperties.data;
    delete[] shaderPaths.data;
    delete[] textureSamplers.data;

    delete[] staticColliders.data;
    delete[] dynamicColliders.data;

    delete[] sfxPaths.data;
    delete[] musicPaths.data;

    delete[] transforms.data;

    delete[] meshRenderers.data;

    delete[] spriteTransforms.data;

    delete[] spriteRenderers.data;

    delete[] instancedSpriteTransforms0.data;

    delete[] instancedSpriteRenderers0.data;

    delete[] spriteInstanceData0;

    delete[] instancedSpriteTransforms1.data;

    delete[] instancedSpriteRenderers1.data;

    delete[] spriteInstanceData1;
}

void ME::Scene::CreateResources() {
    meshPaths.data = new const char*[Constants::MaxMeshCount];
    meshPaths.count = 0;
    quadPaths.data = new const char*[Constants::MaxQuadCount];
    quadPaths.count = 0;
    texturePaths.data = new const char*[Constants::MaxTextureCount];
    texturePaths.count = 0;
    spriteTexturePaths.data = new const char*[Constants::MaxSpriteTextureCount];
    spriteTexturePaths.count = 0;
    textureAtlasProperties.data = new ME::TextureAtlasProperties[Constants::MaxTextureAtlasPropertiesCount];
    textureAtlasProperties.count = 0;
    shaderPaths.data = new const char*[Constants::MaxShaderCount];
    shaderPaths.count = 0;
    textureSamplers.data = new ME::TextureSampler[Constants::MaxSamplerCount];
    textureSamplers.count = 0;
    transforms.data = new ME::Transform[Constants::MaxTransformCount];
    transforms.count = 0;
    meshRenderers.data = new ME::MeshRenderer[Constants::MaxMeshRendererCount];
    meshRenderers.count = 0;
    spriteTransforms.data = new ME::Transform[Constants::MaxSpriteTransformCount];
    spriteTransforms.count = 0;
    spriteRenderers.data = new ME::SpriteRenderer[Constants::MaxSpriteRendererCount];
    spriteRenderers.count = 0;

    instancedSpriteTransforms0.data = new ME::Transform[Constants::MaxInstancedSpriteTransformCount];
    instancedSpriteTransforms0.count = 0;
    instancedSpriteRenderers0.data = new ME::SpriteRenderer[Constants::MaxInstancedSpriteRendererCount];
    instancedSpriteRenderers0.count = 0;
    spriteInstanceData0 = new ME::SpriteRendererInstanceData[Constants::MaxInstancedSpriteRendererCount];

    instancedSpriteTransforms1.data = new ME::Transform[Constants::MaxInstancedSpriteTransformCount];
    instancedSpriteTransforms1.count = 0;
    instancedSpriteRenderers1.data = new ME::SpriteRenderer[Constants::MaxInstancedSpriteRendererCount];
    instancedSpriteRenderers1.count = 0;
    spriteInstanceData1 = new ME::SpriteRendererInstanceData[Constants::MaxInstancedSpriteRendererCount];

    staticColliders.data = new ME::ColliderAABB[Constants::MaxStaticColliderCount];
    staticColliders.count = 0;
    dynamicColliders.data = new ME::ColliderAABB[Constants::MaxDynamicColliderCount];
    dynamicColliders.count = 0;

    sfxPaths.data = new const char*[Constants::MaxLoadedSFXCount];
    sfxPaths.count = 0;
    musicPaths.data = new const char*[Constants::MaxLoadedMusicCount];
    musicPaths.count = 0;
}

void ME::Scene::BuildLights() {
    ambientLight = new ME::Light();
    ambientLight->color = ME::Color::White();
    ambientLight->intensity = 0.04f;

    directionalLight = new ME::Light();
    directionalLight->direction = ME::Vec3(1.0f, 2.0f, -4.0f).Normalised();
    directionalLight->color = ME::Color::White();
    directionalLight->intensity = 1.0f;
}

void ME::Scene::BuildCamera() {
    camera = new ME::Camera();
    camera->position = ME::Vec3(0.0f, 4.0f, -3.0f);
    camera->viewPosition = ME::Vec3(0.0f, 0.0f, 3.0f);
    camera->projectionType = ME::ProjectionType::Perspective;
    camera->fov = 90.0f;
    camera->aspectRatio = 1.33f;

    spriteCamera = new ME::Camera();
    spriteCamera->position = ME::Vec3(0.0f, 0.0f, 0.0f);
    spriteCamera->viewPosition = ME::Vec3(0.0f, 0.0f, 1.0f);
    spriteCamera->projectionType = ME::ProjectionType::Orthographic;
    spriteCamera->orthographicSize = 1000.0f;
    spriteCamera->aspectRatio = 1.33f;
}

void ME::Scene::BuildTransforms() {}

void ME::Scene::BuildMeshRenderers() {}

void ME::Scene::BuildSpriteTransforms() {}

void ME::Scene::BuildSpriteRenderers() {}

void ME::Scene::BuildInstancedSpriteTransforms() {}

void ME::Scene::BuildInstancedSpriteRenderers() {}

const char* ME::Scene::GetDisplayName() const {
    return "Scene";
}

void ME::Scene::Update() {
    UpdateSpriteRenderers();
    UpdateInstancedSpriteRenderers();
}

void ME::Scene::UpdateSpriteRenderers() {
    // Updating transforms and atlas indices for dirty sprites.
    // In separate loops to avoid cache misses.
    for (size_t i = 0; i < spriteRenderers.count; ++i) {
        if (!spriteRenderers[i].bDirty) {
            continue;
        }
        // Updated any required data before rendering.
    }

    for (size_t i = 0; i < spriteRenderers.count; ++i) {
        if (spriteRenderers[i].bDirty) {
            spriteRenderers[i].bDirty = false;
        }
    }
}

void ME::Scene::UpdateInstancedSpriteRenderers() {
    // Updating transforms and atlas indicesfor dirty instances.
    // In separate loops to avoid cache misses.

    for (size_t i = 0; i < instancedSpriteRenderers0.count; ++i) {
        if (!instancedSpriteRenderers0[i].bDirty) {
            continue;
        }
        spriteInstanceData0[i].modelMatrixData = instancedSpriteTransforms0[i].GetModelMatrix().GetDataForShader();
    }

    for (size_t i = 0; i < instancedSpriteRenderers0.count; ++i) {
        if (!instancedSpriteRenderers0[i].bDirty) {
            continue;
        }
        spriteInstanceData0[i].atlasIndex = instancedSpriteRenderers0[i].atlasIndex;
        spriteInstanceData0[i].color = instancedSpriteRenderers0[i].color;
        spriteInstanceData0[i].flags = instancedSpriteRenderers0[i].flags;
    }

    for (size_t i = 0; i < instancedSpriteRenderers0.count; ++i) {
        if (instancedSpriteRenderers0[i].bDirty) {
            instancedSpriteRenderers0[i].bDirty = false;
        }
    }

    for (size_t i = 0; i < instancedSpriteRenderers1.count; ++i) {
        if (!instancedSpriteRenderers1[i].bDirty) {
            continue;
        }
        spriteInstanceData1[i].modelMatrixData = instancedSpriteTransforms1[i].GetModelMatrix().GetDataForShader();
    }

    for (size_t i = 0; i < instancedSpriteRenderers1.count; ++i) {
        if (!instancedSpriteRenderers1[i].bDirty) {
            continue;
        }
        spriteInstanceData1[i].atlasIndex = instancedSpriteRenderers1[i].atlasIndex;
        spriteInstanceData1[i].color = instancedSpriteRenderers1[i].color;
        spriteInstanceData1[i].flags = instancedSpriteRenderers1[i].flags;
    }

    for (size_t i = 0; i < instancedSpriteRenderers1.count; ++i) {
        if (instancedSpriteRenderers1[i].bDirty) {
            instancedSpriteRenderers1[i].bDirty = false;
        }
    }
}
