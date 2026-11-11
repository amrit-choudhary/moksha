# Porting a DX12 feature to Vulkan

Vulkan is the Android renderer (`rendering/vulkan/`, `scene/scene_vulkan.*`, `scene/scene_ui_vulkan.*`,
`shaders/glsl/`). On Windows it is a development-only build (`-DMOKSHA_RENDERER=VULKAN`, preset `win-vulkan-dev`) used
to develop with the validation layer and RenderDoc; whatever runs there must run unchanged on the phone. Workflow and
verification: [README.md](README.md). How it was built, lesson by lesson: [android_port_plan.md](../android_port_plan.md).

## Target: Vulkan 1.1 core, phone limits
- **Vulkan 1.1, no extensions beyond the surface and swapchain.** So: `VkRenderPass` with one `VkFramebuffer` per
  swapchain image (no dynamic rendering), `vkCmdPipelineBarrier` (no synchronization2), no descriptor indexing or
  bindless, shaders compiled with `--target-env=vulkan1.1`. The instance and device ask for 1.1 on Windows too.
- **Only what phones have:** no geometry shaders, wide lines, BC textures or `IMMEDIATE` present mode; `FIFO` present;
  one graphics + present queue; formats queried, never assumed.
- **Design to the spec minimums, not the desktop's numbers:**

  | Limit | Spec minimum (and the test phone) | Desktop | What it shapes |
  |---|---|---|---|
  | `maxBoundDescriptorSets` | 4 | 32 | at most sets 0-3 per pipeline layout |
  | `maxUniformBufferRange` | 16 KB | 64 KB | arrays go in storage buffers, not UBOs |
  | `maxDescriptorSetUniformBuffersDynamic` | 8 | more | dynamic UBOs summed over all sets of a layout |
  | `maxDescriptorSetStorageBuffersDynamic` | 4 | more | dynamic SSBOs summed over all sets of a layout |
  | `minUniformBufferOffsetAlignment` / `minStorageBufferOffsetAlignment` | up to 256 | 64-256 | queried in `UploadBufferVulkan` |

## Class mapping
| DX12 | Vulkan | Notes |
|---|---|---|
| `RendererDX` | `RendererVulkan` | reaches the window only through `VulkanView` (per OS in `platform/<os>/`) |
| `SceneDX`, `SceneUIDX` | `SceneVulkan`, `SceneUIVulkan` | `Init`/`End` returning failure instead of a constructor; own their descriptor pool |
| `RootSigDx::CreateRootSignature*` | `PipelineLayoutVulkan::CreateLayout*` | plus the descriptor set layouts (`CreateDescriptorSetLayout*`) |
| `PSODirectX::CreatePSO*` | `PSOVulkan::CreatePSO*` | same names; caller owns and destroys what they return |
| DX12 static samplers | `SamplerVulkan::Create*`, built into set layouts as immutable samplers | destroyed after the set layouts |
| `UploadBufferDX` | `UploadBufferVulkan` (`Kind::Uniform` / `Kind::Storage`) | one copy per frame in flight |
| `TextureDX` | `TextureVulkan` | device-local, filled through a staging copy |
| `QuadDX`, `ShaderDX` | `QuadVulkan`, `ShaderVulkan` | `shaders/glsl/<name>.vert.spv` / `.frag.spv` through `Vfs` |
| `DescHeapManagerDX` | (none) | descriptor pools per scene; no shared heap |
| `FlushCommandQueue` | `vkDeviceWaitIdle` | before replacing a scene or in `End` |

## Binding model
| DX12 | Vulkan |
|---|---|
| Root signature | Pipeline layout (an ordered list of set layouts) |
| Descriptor table | Descriptor set, made from a set layout |
| One CBV per frame at heap index + frame | One descriptor of type `*_DYNAMIC`; the frame's copy picked by a dynamic offset at bind time |
| Constant buffer (CBV, `b#`) | Uniform buffer (`UNIFORM_BUFFER_DYNAMIC`), GLSL `uniform` block, `std140` |
| `StructuredBuffer` (SRV, `t1`) | Storage buffer (`STORAGE_BUFFER_DYNAMIC`), GLSL `readonly buffer`, `std430` |
| Texture SRV (`t0`) + static sampler (`s1`) | `COMBINED_IMAGE_SAMPLER` with the sampler immutable in the set layout |
| `SetGraphicsRootSignature` (drops all bindings) | Switching layouts keeps set N bound while sets 0..N have identical set layouts |

Sets are grouped by how often they change (the 4-set limit rules out one set per DX12 table):

| Set | Content | Plain sprites | Instanced and UI passes |
|---|---|---|---|
| 0 | per pass | `CBPerPass` (dynamic UBO) | `CBPerPass`, or the UI pass constants |
| 1 | per draw | binding 0 `CBPerSprite`, binding 1 atlas (dynamic UBOs) | binding 0 instance array (dynamic SSBO), binding 1 atlas (dynamic UBO) |
| 2 | texture | combined image sampler | combined image sampler |
| 3 | free | | |

## Rules (each one cost a debugging session)
1. **Dynamic offsets are one per dynamic descriptor across all the sets being bound, in set-then-binding order.**
   Binding sets 1 and 2 with `firstSet = 1` takes set 1's offsets only (`{binding 0, binding 1}`). A wrong count or
   order is a validation error, and on some drivers then `VK_ERROR_DEVICE_LOST`.
2. **A descriptor's range must cover the shader's block.** A std140 struct rounds its size up to 16 bytes, so a
   4-byte C++ constant (`CBPerPassUISprite`) declared as a struct inside a block is too big for its 4-byte range.
   Declare small constants directly in the block (`uint screenDimension;`), or pad the C++ type to 16.
3. **Uniform elements are padded, storage elements are packed.** A dynamic offset must be a multiple of the kind's
   alignment. `UploadBufferVulkan` pads each uniform element to it (each is a descriptor's target) and packs storage
   elements at their own size (the std430 array stride the shader indexes), aligning only each frame's copy. Use
   `GetOffset(frame, i)` for a uniform element, `GetOffset(frame, 0)` for a storage array, and `GetDescriptorRange()`
   for the descriptor.
4. **Instance arrays go in storage buffers.** 16 KB of uniforms holds 170 instances of 96 bytes. The descriptor range is
   the whole capacity; the draw's `instanceCount` limits what the shader reads.
5. **A descriptor pool size with `descriptorCount = 0` is invalid.** Build the `VkDescriptorPoolSize` array with a
   counter and leave out types the scene has none of; size `maxSets` from the same counts.
6. **Descriptor types must match in three places:** the set layout binding, each `VkWriteDescriptorSet`, and the
   shader's declaration. Validation checks the shader against the layout when the pipeline is created, so a mismatch
   shows at start-up.
7. **Bind set 0 where every pass that needs it can see it.** `RendererVulkan::Draw` binds the scene's set 0 once with
   `layout2DAtl`; it stays bound for the instanced passes because their layout has the same set 0 layout. A pass that
   may be skipped must not be the only one that binds a set the next pass relies on. The UI passes bind all of sets
   0-2 themselves.
8. **Y is flipped by the viewport.** Viewport and scissor are dynamic state set once per frame; the viewport has
   `y = height` and a negative `height`, so shaders keep DX12's Y-up NDC. Don't flip in shaders.
9. **Frames in flight:** write a frame's buffer copies only after its fence wait (`SceneVulkan::UploadFrame`,
   `SceneUIVulkan::UploadFrame` run right after it). `vkDeviceWaitIdle` before a scene is replaced
   (`SetScenes`, `SetDebugUIScene`) or anything is destroyed. `End` destroys in reverse creation order, scenes first
   (their sets come from the renderer's set layouts).
10. **Synchronisation objects:** image-available semaphores and fences per frame in flight; render-finished semaphores
    per swapchain image (present waits on them and signals nothing, so only the image's next acquire proves it free).
11. **Depth is mobile-shaped:** format queried (D32, X8_D24, then D16), one image for all frames as a transient
    attachment in lazily allocated memory when the GPU has it, cleared by the load op, `storeOp = DONT_CARE`.
12. **Memory:** buffers the CPU writes use host-visible coherent memory (device-local + host-visible first, for unified
    mobile memory); textures are device-local, uploaded through a staging buffer and one-time commands.

## Shaders (GLSL → SPIR-V)
- One file per stage: `<name>.vert` and `<name>.frag`, `#version 450`, entry point `main`. Both are listed in the
  `glsl` list of `res_list.json`; the cook runs `glslc --target-env=vulkan1.1` into `<file>.spv`.
- `#extension GL_GOOGLE_include_directive : require` and `#include "structs.glsl"` / `"helpers.glsl"` (same folder).
- Bindings are explicit: `layout(std140, set = 0, binding = 0) uniform PerPassBlock { ... };` (mind the `;`),
  `layout(std430, set = 1, binding = 0) readonly buffer InstanceBlock { SpriteInstanceData instances[]; };`,
  `layout(set = 2, binding = 0) uniform sampler2D tex;`.
- HLSL → GLSL: `float4x4` → `mat4`, `mul(v, M)` → `M * v`, `SV_InstanceID` → `gl_InstanceIndex` (which includes
  `firstInstance`; the engine always draws with 0), `clip(-1)` → `discard`, `tex.Sample(s, uv)` → `texture(tex, uv)`,
  `SV_Position` → `gl_Position`, interpolants → `layout(location = n) in/out`.
- Keep `structs.glsl` byte for byte with the C++ structs; the 96-byte instance structs keep their `uvec2 padding`.

## Porting a feature: checklist
1. Read the DX12 change: its root signature, PSO, scene buffers, draw code and HLSL.
2. Shaders: port each HLSL pass to `.vert` / `.frag`, add structs and helpers, list them in `res_list.json`.
3. Bindings: decide the sets (rules above); add set layouts to `PipelineLayoutVulkan` if a set's shape is new.
4. Pipeline: `PipelineLayoutVulkan::CreateLayout<Name>` and `PSOVulkan::CreatePSO<Name>` with DX12's names; blend,
   depth and vertex input as the DX12 PSO, `cullMode = VK_CULL_MODE_NONE` for 2D (no winding question arises under
   the flipped viewport while nothing is culled). Create them in `CreatePSO`, destroy them in `End`.
5. Scene resources: buffers (`UploadBufferVulkan` with the right `Kind`), pool sizes, sets, per-frame upload in
   `UploadFrame`, cleanup in `End`.
6. Draw: bind pipeline, sets with offsets in the right order, `vkCmdDrawIndexed`; skip on empty counts and missing
   slots.
7. Verify: no `Vulkan:` messages in the log, same picture as DX12, then on the phone.
