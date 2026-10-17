#pragma once

/**
 * Scene class implementation
 * This file contains the implementation of the Scene class, which is responsible for managing
 * the 3D scene, including loading models, managing lights, and rendering the scene.
 * The Scene class provides methods to load models, set up lights, and render the scene using
 * the rendering engine. It also handles the camera and view transformations.
 *
 * UI related functionalities are handled in a separate SceneUI class.
 */

#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/light.h"
#include "moksha/client/rendering/shared/material.h"
#include "moksha/client/rendering/shared/mesh_renderer.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/client/rendering/shared/texture.h"
#include "moksha/client/rendering/shared/texture_sampler.h"
#include "moksha/shared/datastructure/span.h"
#include "moksha/shared/physics/collider_aabb.h"

namespace ME {

class Scene {
   public:
    Scene();
    virtual ~Scene();

    /**
     * Updates the rendering related data in the scene, if any render element is marked as dirty.
     * This is called from Renderer.Update(), right before Renderer.Draw().
     * Like if sprite renderer's atlas index changed due to animation.
     * This is called every frame before rendering.
     * Everything that is not DX or MTL dependent is updated here, after which DX and MTL scene is updated from
     * Renderer.Update().
     */
    void Update();

   public:
    ME::Light* ambientLight;
    ME::Light* directionalLight;
    ME::Camera* camera;
    ME::Camera* spriteCamera;

    /**
     * Array of scene resources like meshes, textures, shaders, etc.
     * Items index in the array are file names and are used to create the mesh renderer.
     */
    ME::Span<const char*> meshPaths;
    ME::Span<const char*> quadPaths;
    ME::Span<const char*> texturePaths;
    ME::Span<const char*> spriteTexturePaths;
    ME::Span<const char*> shaderPaths;
    ME::Span<ME::TextureSampler> textureSamplers;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;
    ME::Span<ME::Transform> transforms;
    ME::Span<ME::MeshRenderer> meshRenderers;
    ME::Span<ME::Transform> spriteTransforms;
    ME::Span<ME::SpriteRenderer> spriteRenderers;

    // First set of instanced sprite renderers.
    ME::Span<ME::Transform> instancedSpriteTransforms0;
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers0;
    ME::SpriteRendererInstanceData* spriteInstanceData0 = nullptr;

    // Second set of instanced sprite renderers.
    ME::Span<ME::Transform> instancedSpriteTransforms1;
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers1;
    ME::SpriteRendererInstanceData* spriteInstanceData1 = nullptr;

    ME::Span<ME::ColliderAABB> staticColliders;
    ME::Span<ME::ColliderAABB> dynamicColliders;

    ME::Span<const char*> sfxPaths;
    ME::Span<const char*> musicPaths;

    virtual void Init();
    virtual void CreateResources();
    virtual void BuildLights();
    virtual void BuildCamera();
    virtual void BuildTransforms();
    virtual void BuildMeshRenderers();
    virtual void BuildSpriteTransforms();
    virtual void BuildSpriteRenderers();
    virtual void BuildInstancedSpriteTransforms();
    virtual void BuildInstancedSpriteRenderers();

    // Human-readable label for this scene, shown on the debug overlay. Override to customize.
    virtual const char* GetDisplayName() const;

   private:
    /**
     * Updates sprite renderers that are marked as dirty.
     * After update, the dirty flag is cleared.
     */
    void UpdateSpriteRenderers();

    /**
     * Updates instanced sprite renderers that are marked as dirty.
     * After update, the dirty flag is cleared.
     */
    void UpdateInstancedSpriteRenderers();
};

}  // namespace ME
