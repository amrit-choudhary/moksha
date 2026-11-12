#pragma once
#ifdef ME_WEBGPU

/**
 * Renderer class for WebGPU, mirroring RendererMetal pass for pass. Draws into the view's surface once per Tick. The
 * browser presents the frame when control returns to it, and keeps resources alive for the GPU, so there are no fences
 * or frames in flight here.
 */

#include <webgpu/webgpu.h>

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"
#include "pso_webgpu.h"

namespace ME {

class QuadWebGPU;
class Scene;
class SceneUI;
class SceneUIWebGPU;
class SceneWebGPU;
class UploadBufferWebGPU;
class WebGPUView;
class WindowState;

class RendererWebGPU : public PlatformRenderer {
   public:
    // Configures the view's surface and creates the pipelines. view must outlive the renderer; window is the app's
    // window state.
    bool InitWebGPU(const WebGPUView* inView, const WindowState* inWindow);

    void SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) override;
    void SetDebugUIScene(ME::SceneUI* inDebugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    void Draw() override;

    void End() override;

    // Does nothing: the browser owns the GPU's work.
    void OnPause() override;

    // Does nothing: the browser always presents on the display's refresh.
    void SetVsyncEnabled(bool enabled) override;

   private:
    ME::Scene* scene = nullptr;
    ME::SceneWebGPU* sceneWebGPU = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUIWebGPU* uiSceneWebGPU = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    ME::SceneUIWebGPU* debugUiSceneWebGPU = nullptr;

    const WebGPUView* view = nullptr;
    const WindowState* window = nullptr;

    // Device, queue, shared layouts and the sampler.
    WebGPUContext context;

    // sRGB view of the surface's format, which the renderer draws through.
    WGPUTextureFormat renderFormat = WGPUTextureFormat_Undefined;

    WGPUPipelineLayout perSpritePipelineLayout = nullptr;
    WGPUPipelineLayout instancePipelineLayout = nullptr;

    // Pipeline for 2D rendering with texture atlases. No instancing.
    WGPURenderPipeline pso2DAtl = nullptr;
    // Pipeline for 2D rendering with instancing and texture atlases.
    WGPURenderPipeline pso2DInsAtl = nullptr;
    // The same, blended additively, for instanced sets set to SpriteBlend::Additive.
    WGPURenderPipeline pso2DInsAtlAdditive = nullptr;
    // Pipeline for UI Text rendering.
    WGPURenderPipeline pso2DUIText = nullptr;
    // Pipeline for UI Sprite rendering.
    WGPURenderPipeline pso2DUISprite = nullptr;

    // Depth buffer at the surface's size, which never changes.
    WGPUTexture depthTexture = nullptr;
    WGPUTextureView depthView = nullptr;

    ME::QuadWebGPU* quad = nullptr;

    // Sends the frame's constants and instance data to the GPU, ahead of the draws that read them.
    void UploadSceneData();
    void UploadUIScene(ME::SceneUIWebGPU* uiSceneGPU);

    // Issues the plain sprites, one draw each.
    void DrawSprites(WGPURenderPassEncoder encoder);

    // Issues one instanced sprite set: its texture and atlas slot, instance data, count and blend. Shared by both sets.
    void DrawInstancedSpriteSet(WGPURenderPassEncoder encoder, uint32_t textureIndex, WGPUBindGroup instanceBindGroup,
                                uint32_t instanceCount, ME::SpriteBlend blend);

    // Issues the UI Sprite + UI Text draw passes for one SceneUIWebGPU. Shared by the main UI scene and the debug
    // system's UI scene: same pipelines, different data. No-op if uiSceneGPU is nullptr.
    void DrawUIScene(WGPURenderPassEncoder encoder, ME::SceneUIWebGPU* uiSceneGPU);
};

}  // namespace ME

#endif  // ME_WEBGPU
