#pragma once
#ifdef ME_WEBGPU

/**
 * Render pipelines and the bind group layouts they share. WebGPU bakes depth state into the pipeline, where Metal keeps
 * it apart. Bind groups, matching the @group attributes in shaders/wgsl/:
 *   0: per-pass constants (binding 0), texture atlas properties at a dynamic offset (binding 1)
 *   1: texture (binding 0), its sampler (binding 1)
 *   2: per-sprite constants at a dynamic offset, or the instance data (binding 0)
 */

#include <webgpu/webgpu.h>

namespace ME {

class UploadBufferWebGPU;

// The objects every scene's GPU resources are made from. The renderer owns them.
class WebGPUContext {
   public:
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
    WGPUBindGroupLayout passLayout = nullptr;
    WGPUBindGroupLayout textureLayout = nullptr;
    WGPUBindGroupLayout perSpriteLayout = nullptr;
    WGPUBindGroupLayout instanceLayout = nullptr;
    // Clamped point sampling for pixel art style atlases (the DX static sampler s1).
    WGPUSampler sampler = nullptr;
};

class PSOWebGPU {
   public:
    static constexpr WGPUTextureFormat DepthFormat = WGPUTextureFormat_Depth32Float;

    // Layouts of the bind groups above. Group 2 has one for plain sprites and one for instanced passes.
    static WGPUBindGroupLayout CreatePassLayout(WGPUDevice device);
    static WGPUBindGroupLayout CreateTextureLayout(WGPUDevice device);
    static WGPUBindGroupLayout CreatePerSpriteLayout(WGPUDevice device);
    static WGPUBindGroupLayout CreateInstanceLayout(WGPUDevice device);

    // Groups 0, 1 and objectLayout as group 2.
    static WGPUPipelineLayout CreatePipelineLayout(WGPUDevice device, WGPUBindGroupLayout passLayout,
                                                   WGPUBindGroupLayout textureLayout, WGPUBindGroupLayout objectLayout);

    // A 2D atlas pipeline from shaderName's VS and PS, alpha blended into colorFormat. depthTest is the sprite passes'
    // Less with writes; without it, as in the UI passes, everything draws and writes no depth. Null if the shader file
    // can't be read; errors in the shader or pipeline arrive through the device's error callback.
    static WGPURenderPipeline CreatePSO2D(WGPUDevice device, WGPUPipelineLayout layout, const char* shaderName,
                                          WGPUTextureFormat colorFormat, bool depthTest);

    // Bind groups for the layouts above. The texture one is null when view is, so a failed texture isn't drawn.
    static WGPUBindGroup CreatePassBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* perPassBuffer,
                                             const UploadBufferWebGPU* atlasBuffer);
    static WGPUBindGroup CreateTextureBindGroup(const WebGPUContext& context, WGPUTextureView view);
    static WGPUBindGroup CreatePerSpriteBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* buffer);
    static WGPUBindGroup CreateInstanceBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* buffer);
};

}  // namespace ME

#endif  // ME_WEBGPU
