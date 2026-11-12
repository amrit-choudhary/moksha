# Porting rendering features between graphics backends

The engine has four renderers: DX12 (Windows), Metal 4 (macOS, iOS), Vulkan 1.1 (Android; a development-only build on
Windows) and WebGPU (web). This folder says how a rendering feature moves from one to the others. Per-backend guides:
[metal.md](metal.md), [vulkan.md](vulkan.md), [webgpu.md](webgpu.md).

## Workflow
1. **Build it on DX12 first** (`rendering/directx/`, `scene/scene_dx.*`, `scene/scene_ui_dx.*`, `shaders/hlsl/`), and
   test it there with the samples.
2. **Port it to Metal, Vulkan and WebGPU**, in any order, one backend per change. A port mirrors the DX12 code: the same
   class and function names with the backend's suffix (`CreatePSO2DUISprite` → `PSOVulkan::CreatePSO2DUISprite`), the
   same fields (`instanceCount0`, `spriteInstanceBuffer1`), the same pass order and fixed slots. Only what the API
   forces is different, and each guide lists those differences.
3. **Port the shader file for file.** Every pass exists once per language (`shaders/hlsl/`, `metal/`, `glsl/`, `wgsl/`)
   and includes the language's `structs` and `helpers`. Each header comment names the HLSL file it ports ("keep them
   in sync"). Add new files to `resources/res_list.json` in every language's list.
4. **Shared data first.** Constant and instance layouts live once in C++ (`rendering/shared/shader_constants.h`,
   `sprite_renderer.h`, `text_renderer.h`) and are mirrored by each language's `structs` file. A new struct, or a field
   added to one, changes all five places.
5. **Structural improvements go everywhere, later.** A better structure found in one backend (buffer layout,
   ownership, lifecycle, failure handling) is added to the backport list
   ([android_port_plan.md](../android_port_plan.md), "Backport list") instead of being applied to the other
   backends right away (rule: [moksha/client/CLAUDE.md](../../moksha/client/CLAUDE.md), "Backends stay in step").

## Engine-wide rules a port keeps
- **Backends switch on the graphics API define** (`ME_DX12`, `ME_METAL`, `ME_VULKAN`, `ME_WEBGPU`), never on the OS.
- **Matrices:** C++ hands every backend `Mat4::GetDataForShader()`: row-major for DX12, column-major for the others.
  HLSL multiplies `mul(v, M)`; Metal, GLSL and WGSL multiply `M * v` (`projection * view * model * position`).
- **Clip space:** depth 0..1 everywhere. Y goes up in NDC on every backend; Vulkan gets there with a negative viewport
  height (see [vulkan.md](vulkan.md)). UVs are flipped in the vertex shader (`1 - uv.y`), as in the HLSL.
- **Fixed slots:** game scene instanced set 0 uses texture and atlas 1, set 1 uses 2. A UI scene's atlas is texture and
  atlas 0, its font is 1. A pass whose slot is missing or failed to load is skipped, never read out of range.
- **Pass order:** plain sprites, instanced set 0, instanced set 1, then the game's UI (sprites, then text), then the
  debug overlay. Scene passes test and write depth (`LESS`); UI passes neither test nor write it.
- **No face culling in 2D pipelines** (counter-clockwise is still the front face): a sprite mirrored by a negative
  scale must still draw. 3D pipelines, when they come, may cull back faces.
- **Blending:** every 2D pass blends straight alpha: color `SRC_ALPHA` / `ONE_MINUS_SRC_ALPHA`, alpha `ONE` / `ZERO`,
  both `ADD`. Fragments with alpha below 0.1 are discarded. The one exception is an instanced set whose
  `instancedSpriteBlend` is `SpriteBlend::Additive`: it draws with a second instanced pipeline (`pso2DInsAtlAdditive`,
  same shader and layout), color `SRC_ALPHA` / `ONE`, alpha `ZERO` / `ONE`, depth tested (`LESS`) but not written.
- **Instance counts:** each instanced pass copies and draws `Scene::GetInstancedDrawCount0/1()` instances (the first N
  of the set), never the set's whole capacity.
- **Frames in flight:** `Constants::FramesInFlight` on DX12, Metal and Vulkan: one copy of each upload buffer per
  frame, textures shared, and the GPU drained (`FlushCommandQueue`, `vkDeviceWaitIdle`) before anything a frame used
  is freed or a scene is replaced. WebGPU has no frames in flight (the browser presents and the queue copies writes).
- **No exceptions, RTTI or `std::vector`** in backend code (root [CLAUDE.md](../../CLAUDE.md)). Fixed arrays sized by
  `engine_limits.h`, failures returned and logged.

## Verifying a port
A port is checked by behaviour, in this order:
1. **No API errors.** DX12 with the debug layer; Metal with `MTL_DEBUG_LAYER=1`; Vulkan with the validation layer (debug
   builds load it when found: the log must show no `Vulkan:` warnings or errors); WebGPU with no `WebGPU:` lines in the
   browser console.
2. **Same picture as DX12.** Run the same sample on both and compare screenshots: positions, tiles, tint, blending,
   depth order, UI and text.
3. **Edge cases:** a scene with only instanced sprites, a scene with no UI text, a texture that fails to load (the
   pass is skipped with a log line), UI elements added after start-up.
4. **On the device last.** Vulkan on the Android phone, Metal on iOS, WebGPU in Chrome and Safari.

## Feature parity
What each backend draws today. Update this table in the same change that adds a feature to a backend.

| Feature | DX12 | Metal | Vulkan | WebGPU |
|---|---|---|---|---|
| Plain sprites (per-sprite constants) | yes | yes | yes | yes |
| Instanced sprite sets 0 and 1 | yes (set 1 skipped when set 0 is empty: known issue) | yes | yes | yes |
| Texture atlases (`GetAtlasUV`, flips) | yes | yes | yes | yes |
| Straight alpha blending | yes | yes | yes | yes |
| Additive instanced sets (`SpriteBlend::Additive`) | yes | yes (untested) | yes (Windows dev build; not yet on a phone) | yes (Chrome on Windows) |
| Instanced draw count (copy and draw the first N) | yes | yes (untested) | yes (Windows dev build; not yet on a phone) | yes (Chrome on Windows) |
| Depth buffer for the scene | yes | yes (memoryless) | yes (transient, lazily allocated when available) | yes |
| UI sprite pass | yes | yes | yes | yes |
| UI text pass | yes | yes | yes | yes |
| Debug overlay | yes | yes | yes | yes |
| UI elements added at run time | yes | yes | yes | yes |
| Frames in flight | yes | yes | yes | n/a (browser) |
| Vsync toggle | yes | yes (macOS) | no (always `FIFO`) | n/a (display rate) |
| Window resize / swapchain recreation | no | no | no (lesson 9) | no (canvas fixed) |
| Surface lost and recreated (mobile) | n/a | iOS: pause only | no (lesson 9) | n/a |
| No face culling in 2D passes | yes | yes | yes | yes |

3D meshes are not part of the 2D feature set: DX12 has unused mesh code (`MeshDX`, `CreatePSO3D`) that no other
backend mirrors.
