# Porting a DX12 feature to Metal

Metal is the macOS and iOS renderer (`rendering/metal/`, `scene/scene_metal.*`, `scene/scene_ui_metal.*`,
`shaders/metal/`). It uses **Metal 4 only** (Apple silicon, macOS / iOS 26+). Its model matches DX12's closely, so
Metal classes mirror the DX12 ones class for class. Workflow and verification: [README.md](README.md).

## Metal 4 only
Use `MTL4CommandQueue`, command allocators, argument tables, residency sets, explicit barriers and `MTL4Compiler`.
Never the older per-encoder binding style (`setVertexBytes`, `setVertexBuffer`, `setFragmentTexture`): a port that
reaches for those is porting to the wrong API. metal-cpp is vendored (`third_party/`); the `MTKView` is reached only
through `rendering/metal/metal_view.h`.

## Class mapping
| DX12 | Metal | Notes |
|---|---|---|
| `RendererDX` | `RendererMetal` | reaches the view only through `MetalView` (per OS: `platform/mac/`, `platform/ios/`) |
| `SceneDX`, `SceneUIDX` | `SceneMetal`, `SceneUIMetal` | add every buffer and texture to the residency set |
| `RootSigDx::CreateRootSignature*` | (none) | shaders read fixed argument table indices (`MetalBindings`) |
| `PSODirectX::CreatePSO*` | `PSOMetal::CreatePSO*` | same names; built through `MTL4Compiler` |
| Depth state inside the PSO | `PSOMetal::CreateDepthStateDefault` / `CreateDepthStateDisabled` | set on the encoder per pass |
| DX12 static samplers | `constexpr sampler` in the shader | e.g. point clamp: `filter::nearest, address::clamp_to_edge` |
| `UploadBufferDX` | `UploadBufferMetal` | same constructor shape (`isConstantBuffer`, count, size) and `CopyData` / `CopyElements` |
| `TextureDX`, `QuadDX`, `ShaderDX` | `TextureMetal`, `QuadMetal`, `ShaderMetal` | shaders load the cooked `.metallib` through `Vfs` |
| `DescHeapManagerDX` | (none) | resources are bound by GPU address or resource ID |
| Command allocator per frame, fence | `MTL4::CommandAllocator` per frame, a shared event | `FlushCommandQueue` waits on the event |

## Binding model
Metal has no root signature or descriptor heap. One argument table holds the bindings at fixed indices, and Metal
snapshots it at each draw, so a pass changes only what differs before its next draw.

| DX12 register | `MetalBindings` index | Bound with |
|---|---|---|
| `b0` per-pass constants | buffer 0 `PerPassBuffer` | `argumentTable->setAddress(buffer->GetGPUAddress(frameIndex), index)` |
| `b1` atlas properties | buffer 1 `AtlasBuffer` | same |
| `b2` per-sprite constants | buffer 2 `PerObjectBuffer` | same |
| `t1` instance data (`StructuredBuffer`) | buffer 3 `InstanceBuffer` | same |
| input assembler vertex buffer | buffer 4 `VertexBuffer` | same; the vertex descriptor's layout uses this index |
| `t0` texture | texture 0 `Texture` | `argumentTable->setTexture(texture->gpuResourceID(), index)` |

A new kind of binding gets a new `MetalBindings` constant and the matching `[[buffer(n)]]` / `[[texture(n)]]` in the
shader; keep the comment saying which DX12 register it stands for.

## Rules
1. **Residency:** every buffer and texture the GPU reads must be in the renderer's residency set
   (`residencySet->addAllocation`, then `commit()`), or it reads nothing. Scenes add theirs on creation; the set is
   added to the command queue along with the layer's.
2. **Metal 4 doesn't keep resources alive for the GPU.** Call `FlushCommandQueue` before freeing anything a frame in
   flight may use, and before replacing a scene (`SetScenes`, `SetDebugUIScene`).
3. **Frames in flight:** one command allocator per frame, one copy of each `UploadBufferMetal`'s data per frame
   (`GetGPUAddress(frameIndex)`), a shared event signalled per frame; `Draw` ends by waiting for the next frame's slot.
   Textures are shared.
4. **Constant buffers use a 256-byte stride**, as DX12's; instance buffers are packed.
5. **Depth state is not in the pipeline:** call `encoder->setDepthStencilState` with the default state (`LESS`,
   writes) for scene passes and the disabled state for UI passes. Cull mode and winding are also encoder state, set
   once per frame: counter-clockwise front, `CullModeNone` (2D culls nothing).
6. **Struct layouts:** Metal's `float3` is 16 bytes; use `packed_float3` where C++ has a 12-byte `Vec3`
   (`LightDataDirectional`). A struct holding a `float4x4` has a 16-byte-aligned stride, so the C++ instance data
   classes carry explicit padding to 96 bytes; keep it when adding fields.
7. **The drawable exists only inside draw:** `MEView` hands one out in `drawRect:`, so `Tick` runs from there and
   waiting for the drawable paces the loop. Don't hold a drawable across frames.
8. **Depth** is a renderer-owned memoryless `Depth32Float` texture sized to the drawable.
9. **Validation:** run with `MTL_DEBUG_LAYER=1`.

## Shaders (MSL)
- One `.metal` file per pass, with `VS` and `PS` functions, including `structs.metal` and `helpers.metal` from the
  same folder. Listed in the `metal` list of `res_list.json`; the cook compiles each to a `.metallib` with Xcode's
  Metal compiler (macOS 26 minimum; a separate `ios-metal` cook for iOS devices). Nothing compiles at run time.
- HLSL → MSL: `cbuffer` → `constant T& name [[buffer(n)]]`, `StructuredBuffer<T>` → `device const T* name
  [[buffer(n)]]`, `Texture2D` → `texture2d<float> tex [[texture(0)]]`, `SV_InstanceID` → `uint id [[instance_id]]`,
  vertex inputs → `[[attribute(n)]]` with `[[stage_in]]`, `SV_Position` → `[[position]]`, `mul(v, M)` → `M * v`,
  `clip(-1)` → `discard_fragment()`.
- Indices in the shader must match `MetalBindings`; each shader's comment names the DX12 registers they stand for.

## Porting a feature: checklist
1. Read the DX12 change: root signature (which registers), PSO (blend, depth, input), scene buffers, draw code, HLSL.
2. Shader: port to `.metal`, update `structs.metal` / `helpers.metal`, list it in `res_list.json`.
3. Bindings: map each new register to a `MetalBindings` index (add one if it's a new kind).
4. Pipeline: `PSOMetal::CreatePSO<Name>` with DX12's name; pick the depth state it needs.
5. Scene resources: `UploadBufferMetal` / `TextureMetal`, added to the residency set, per-frame copies.
6. Draw: set the argument table entries, depth state, `drawIndexedPrimitives`; skip on empty counts and missing slots.
7. Verify with `MTL_DEBUG_LAYER=1`, compare with DX12, then run on an iOS device.
