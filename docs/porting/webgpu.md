# Porting a DX12 feature to WebGPU

WebGPU is the web renderer (`rendering/webgpu/`, `scene/scene_webgpu.*`, `scene/scene_ui_webgpu.*`,
`shaders/wgsl/`), built with Emscripten's `emdawnwebgpu` port. It mirrors `RendererMetal` pass for pass, which mirrors
`RendererDX`. Workflow and verification: [README.md](README.md). Background: [web_port_notes.md](../web_port_notes.md).

## What the browser changes
- **The browser presents and keeps resources alive**, and `wgpuQueueWriteBuffer` copies the data at the call. So there
  are no frames in flight, no fences and no `FlushCommandQueue`: an upload buffer holds one copy, and a scene can be
  released and replaced directly.
- **Never call `wgpuSurfacePresent`** (it aborts in the browser); the frame is shown when the main loop callback returns.
- **Errors are asynchronous.** They arrive through the device's error callback and are logged with a `WebGPU:` prefix,
  not as return values; a pipeline or bind group that is wrong still returns a handle.
- **Single-threaded, no Asyncify:** the adapter and device are requested by callback in `platform/web/web_main.cpp`
  before `ClientApp::Init`; renderer code never waits on a callback.
- **The canvas is drawn through an sRGB view** of its preferred format (`viewFormats`), so blending happens in linear
  space as on the other backends.

## Class mapping
| DX12 | WebGPU | Notes |
|---|---|---|
| `RendererDX` | `RendererWebGPU` | reaches the canvas through `WebGPUView`, filled by the driver once it has a device |
| `SceneDX`, `SceneUIDX` | `SceneWebGPU`, `SceneUIWebGPU` | build their bind groups once, at creation |
| `RootSigDx::CreateRootSignature*` | `PSOWebGPU::Create*Layout`, `CreatePipelineLayout` | bind group layouts shared through `WebGPUContext` |
| `PSODirectX::CreatePSO*` | `PSOWebGPU::CreatePSO2D(..., depthTest)` | depth state baked into the pipeline |
| DX12 static samplers | `WebGPUContext::sampler` in bind group 1 | point clamp |
| `UploadBufferDX` | `UploadBufferWebGPU` | constants: `SetElement` + `Upload`; instances: `WriteElements` |
| `TextureDX`, `QuadDX`, `ShaderDX` | `TextureWebGPU`, `QuadWebGPU`, `ShaderWebGPU` | WGSL source compiled by the browser |
| `DescHeapManagerDX` | (none) | bind groups hold the resources |

## Binding model
Bind groups, matching the `@group` / `@binding` attributes in `shaders/wgsl/` (see `pso_webgpu.h`):

| Group | Binding 0 | Binding 1 |
|---|---|---|
| 0 per pass | per-pass constants (`CBPerPass`, or `CBPerPassUI`) | atlas properties, dynamic offset |
| 1 texture | `texture_2d<f32>` | sampler |
| 2 per draw | per-sprite constants (dynamic offset), or the instance array (`var<storage, read>`) | |

This grouping differs from Vulkan's (where the atlas sits in set 1 and the texture in set 2): each backend groups by
its own constraints, and only the shaders and `PSOWebGPU` need to agree.

## Rules
1. **Dynamic offsets are 256-byte aligned** (`minUniformBufferOffsetAlignment`): constant buffers hold elements at a
   256-byte stride and pass `GetOffset(i)` when setting the group. Offsets are given in binding order within a group.
2. **Uniform arrays and structs have a 16-byte stride rule**, so WGSL structs declare only the fields the shader reads;
   the C++ padding arrays can't be mirrored as arrays. A buffer may be bigger than its WGSL struct.
3. **Instance arrays are storage buffers** written straight from the caller's packed array (`WriteElements`, count
   clamped to capacity). WebGL2 has no storage buffers, which is one reason the web port is WebGPU-only.
4. **Bind groups are made once, with the scene.** Anything that changes per frame goes through buffer writes and
   dynamic offsets, never a new bind group per draw. A texture that failed to load gets no bind group, and its draws
   are skipped.
5. **Depth** is `Depth32Float`, baked into each pipeline: `depthTest` true for scene passes (`LESS` with writes), false
   for UI passes. Cull state is baked in too: `frontFace = CCW`, `cullMode = None` (2D culls nothing).
6. **Limits** (WebGPU defaults, what every browser guarantees): at most 4 bind groups, 64 KB per uniform binding,
   128 MB per storage binding. The device is requested without raised limits; design within the defaults.

## Shaders (WGSL)
- One `.wgsl` file per pass with `VS` and `PS` entry points, listed in the `wgsl` list of `res_list.json`. The cook
  keeps it as source and replaces each `#include "file"` line with that file from the shader's folder (WGSL itself has
  no include), so `structs.wgsl` and `helpers.wgsl` are joined into every pass.
- HLSL → WGSL: `cbuffer` → `@group(g) @binding(b) var<uniform> name: T;`, `StructuredBuffer<T>` →
  `var<storage, read> name: array<T>;`, `Texture2D` / `SamplerState` → `texture_2d<f32>` / `sampler`,
  `tex.Sample(s, uv)` → `textureSample(tex, s, uv)`, `SV_InstanceID` → `@builtin(instance_index)`, `float4x4` →
  `mat4x4f`, `mul(v, M)` → `M * v`, `clip(-1)` → `discard`.
- Errors in a shader arrive through the device's error callback as a `WebGPU:` log line, not as a failed call.

## Porting a feature: checklist
1. Read the DX12 change: root signature, PSO, scene buffers, draw code, HLSL.
2. Shader: port to `.wgsl`, update `structs.wgsl` / `helpers.wgsl` with only the fields read, list it in
   `res_list.json`.
3. Bindings: fit the new data into groups 0-2 (or a new group layout in `PSOWebGPU`), with dynamic offsets where a
   draw picks one element.
4. Pipeline: `PSOWebGPU::CreatePSO2D` with the right layout and `depthTest`.
5. Scene resources: `UploadBufferWebGPU`, bind groups at creation, writes per frame.
6. Draw: set pipeline, groups with offsets, `wgpuRenderPassEncoderDrawIndexed`; skip on empty counts and missing slots.
7. Verify: no `WebGPU:` lines in the console, same picture as DX12, in Chrome and Safari.
