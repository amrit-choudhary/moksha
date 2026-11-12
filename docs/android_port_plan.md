# Android port (Vulkan): plan

Status (2026-11-10): Part W done (a sample opens a blank window on Windows Vulkan). Lessons 0-5 done (instance,
validation, messenger, GPU pick, surface, queue family, device, swapchain, render pass, framebuffers, command buffers,
sync objects, a frame that clears and presents; GLSL shaders, `ShaderVulkan`, `QuadVulkan`, `PSOVulkan`, a gradient
quad; `UploadBufferVulkan`, descriptor set layouts, pool and sets, dynamic offsets, the scene camera, Y-flip viewport);
Vulkan 1.1 chosen; lesson 6 textures; lesson 7a depth buffer; 7b sprite_atlas shaders; 7c `SceneVulkan` and plain sprites; 7d instanced sets (all scene sprites draw). 8a blending; 8b-8d UI passes and debug overlay (ported). Lessons 0-8 done. DX12-alignment pass done. Porting guides written (`docs/porting/`). A0 done on Windows 2026-11-13 (Mac later). A1 done 2026-11-13. A2 + A3 done 2026-11-13. Next: A4. After the lessons: write the porting guides (see below). To resume, start a fresh session
with "Read docs/android_port_plan.md and continue from the status table". Claude updates the table at the end of each
step.

| Step | Who | Where | State |
|---|---|---|---|
| Plan | Claude | Mac | done 2026-11-07 |
| W1: `ME_VULKAN` build config, `VulkanView`, `WinVulkanView`, `RendererVulkan` skeleton, `ClientApp`/`win_main` | Claude | Windows | done 2026-11-07 |
| W2: `win-vulkan` cook, `moksha_add_game` mapping, `win-vulkan-dev` preset (sample opens a blank window) | Claude | Windows | done 2026-11-07 |
| Lessons 0-8 | user | Windows | lessons 0-6 done 2026-11-10 (lesson 6: the UI atlas drawn on the test quad, crisp and right way up; `TextureVulkan`, `SamplerVulkan` with immutable samplers, set 2). Lesson 7 split into 7a depth buffer, 7b finish `sprite_atlas` shaders (atlas UVs, color, discard), 7c `SceneVulkan` + plain sprites, 7d instanced sets via storage buffers. 7a done 2026-11-11 (format queried D32/X8_D24/D16, transient attachment in lazily allocated memory when the GPU has it, `storeOp` DONT_CARE, one depth image ordered by the subpass dependency). Sets for 7b-7d: 0 per pass, 1 per draw (sprite UBO or instance SSBO + atlas properties, dynamic offsets), 2 texture; set 3 free. 7b done 2026-11-11 (`helpers.glsl` `GetAtlasUV`, color tint, alpha discard; set 1 binding 1 atlas properties; test quad shows one tinted tile). Lesson learned: dynamic offsets are one per dynamic descriptor across all bound sets, in set-then-binding order; a mismatch shows as a validation error and then `VK_ERROR_DEVICE_LOST`. 7c done 2026-11-11 (`SceneVulkan`: sprite textures, one buffer for all sprites and one for all atlases, its own pool and sets with a set 2 per texture, made by `SetScenes` after a device wait; `DrawSprites` binds set 0 once, then sets 1 and 2 per sprite with `firstSet` 1, skipping out-of-range ids; the test quad is gone; samples' plain sprites match DX12). 7d done 2026-11-12 (`sprite_instanced_atlas` GLSL: std430 storage block read by `gl_InstanceIndex`; `UploadBufferVulkan::Kind` uniform or storage, an instance array being one element so it stays packed; set 1 for instancing = dynamic SSBO + atlas UBO; `CreateLayout2DInstancedAtlas` / `CreatePSO2DInstancedAtlas` (DX12 names); `SceneVulkan` makes each instanced set on its own, DX12-shaped fields `...0`/`...1`; set 0 bound once in `Draw`, kept across both atlas layouts by layout compatibility; `DrawInstancedSprites` with texture/atlas slots 1 and 2; samples' sprites match DX12 except blending). Lesson 7 done. Lesson 8: 8a blending (DX12's scene and UI pipelines blend `SRC_ALPHA`/`INV_SRC_ALPHA`; the Vulkan PSOs don't yet), then `SceneUIVulkan` (UI sprites, text, debug overlay). 8a done 2026-11-12 (both scene PSOs, kept as two functions; samples' game scenes match the WebGPU builds). 8b-8d written by Claude as a port of `SceneUIDX` / `DrawUIScene` 2026-11-12 (the user judged them repeats of lesson 7): `ui_sprite` / `ui_text` GLSL, `SceneUIVulkan`, `UploadBufferVulkan::CopyElementBytes`, `CreatePSO2DUISprite` / `CreatePSO2DUIText` (depth off), the debug overlay; done 2026-11-12 (samples' UI and text draw). **Lessons 0-8 done.** DX12-alignment pass on the UI port written 2026-11-12 (own UI pipeline layouts, two full UI PSO functions, written-out passes in `DrawUIScene`, `UploadBufferVulkan::CopyElements` by element count with packed storage elements and `GetDescriptorRange`), done 2026-11-12 (samples unchanged). Next: porting guides, then Part A |
| Part A: Android platform | Claude | Windows (later also Mac) | A0 started 2026-11-12: `android-arm64` preset added (Ninja, NDK toolchain from `ANDROID_NDK_HOME`, `arm64-v8a`, `android-29`, `MOKSHA_COOK_EXECUTABLE` from `MOKSHA_HOST_COOK`). A0 done on Windows 2026-11-13: packages installed with the Android CLI (`winget install --id Google.AndroidCLI`; `sdkmanager` is deprecated, so no `cmdline-tools`), SDK root `D:\Android\Sdk` set by `--sdk=` in `%USERPROFILE%\.androidrc`; `platform-tools`, `build-tools/37.0.0`, `platforms/android-37.2`, `ndk/30.0.16248370` (r30, the LTS); JDK 21 (Temurin); `ANDROID_HOME`, `ANDROID_NDK_HOME`, `MOKSHA_HOST_COOK` (the `dev` build's `Moksha_Cook.exe`) set per user; `adb devices` lists the phone and `cmake --preset android-arm64` configures. Mac: A0 still to do, same packages. A1 written 2026-11-13: VULKAN default on Android; `Moksha_Client` links `vulkan` (NDK sysroot, no `find_package`) and `android log`; `Moksha_Entry` = `platform/android/*.cpp` (none yet; A5) + the NDK's `android_native_app_glue.c` (an OBJECT library keeps it, so no `-u ANativeActivity_onCreate`); headless filter excludes `input_manager_android`; miniaudio links `dl m`; the engine's `android-arm64` preset turns tests and tools off. Done 2026-11-13: `cmake --build --preset android-arm64` builds the engine libraries (the renderer from lessons 0-8 compiles for Android unchanged). A2 + A3 written together 2026-11-13 (the APK needs cooked assets): cook `android-vulkan` = `win-vulkan`, `glslc` from the NDK's `shader-tools/<host>`; `moksha_add_game` on Android makes `lib<target>.so` and, after each link, `<target>.apk` (aapt2 link with `--debug-mode` in Debug, `jar --update --no-compress` adds `lib/<abi>/`, `zipalign -P 16`, `apksigner` with a keystore `keytool` makes once in the build folder); tools found from `ANDROID_HOME` (newest build-tools and platform unless `MOKSHA_ANDROID_BUILD_TOOLS` / `MOKSHA_ANDROID_JAR` are set) and `JAVA_HOME`; target SDK from the platform; the app id as on iOS with `_` for invalid characters; `<target>_install`; no headless twin or servers on Android; a stub `android_main` (the glue needs one to link). Done 2026-11-13: a sample's APK installs and opens a black screen. The samples' `android` preset sets `MOKSHA_BUNDLE_ID_PREFIX` (`com.moksha.samples`) and cooks with `build-live`'s `Moksha_Cook`. Next: A4 The samples got an `android` preset in their per-machine `CMakeUserPresets.json` (inherits `live`, `build-android/`, cook from `build-live`, tests off, like `web`); A7 documents it in the samples' `CLAUDE.md` and adds it on the Mac. Next: A1 |
| Lesson 9 | user | phone | — |
| Porting guides (`docs/porting/`) | Claude | any | done 2026-11-12: README (workflow, engine-wide rules, verification, feature parity table) and one DX12 → API guide each for Metal, Vulkan, WebGPU (mapping, binding model, hard rules, shader translation, checklist) |
| Backport list (below) to DX12, Metal, WebGPU | user + Claude | each backend's machine | backlog: **Claude reminds the user of open items at the end of each step**; applied only when the user asks |

**Backport list.** A backlog of structural improvements found during the Vulkan port that belong in the other backends
too (rule: [moksha/client/CLAUDE.md](../moksha/client/CLAUDE.md), "Backends stay in step"). Claude adds each one when
it is found, reminds the user of the open items at the end of each step, and changes nothing until the user asks.

| # | Improvement | DX12 | Metal | WebGPU |
|---|---|---|---|---|
| 1 | The backend scene doesn't own the game scene's lights and cameras: `~SceneDX` deletes the ones `~Scene` also deletes (a double free on scene change); read them through the `Scene` pointer instead of copying them | fix | ok (no delete) | ok |
| 2 | One buffer for all per-sprite constants and one for all atlas properties, each draw picking its element by offset, instead of one upload buffer per sprite and per atlas | to do | to do | ok (already) |
| 3 | The backend scene has `Init`/`End` returning failure, instead of a constructor that can't report it | to do | to do | to do |
| 4 | A sprite whose `textureId` is out of range is skipped, not read past the texture array | to do | check | ok (already) |
| 5 | Free a texture's CPU pixel copy (`Texture::data`) once its upload is done, e.g. a `Texture::ReleaseCpuData()` each backend calls after uploading; today every loaded texture keeps its decoded pixels in RAM for life (~1.3 MB for the UI atlas). Shared `Texture` code, so Vulkan too | to do | to do | to do (+ Vulkan) |
| 6 | One cull rule for every 2D pipeline: **no culling** (decided 2026-11-12; culling gains a 2D quad nothing and would hide a sprite mirrored by a negative scale). Vulkan already used `VK_CULL_MODE_NONE`; DX12, Metal and WebGPU culled back faces. DX12's unused `CreatePSO3D` keeps back-face culling | done 2026-11-12 | done 2026-11-12 | done 2026-11-12 (Vulkan: already) |

**Long-term workflow (decided 2026-11-10):** the user's primary backends are DX12 and Metal. New rendering features are
built and tested on DX12 first; once they work, they are ported (by AI) to Metal, Vulkan and WebGPU. Ports are checked
by behavior: zero validation (Vulkan) or `WebGPU:` errors, a screenshot matching DX12, and the phone last. To make ports
smooth, once the lessons are done Claude writes `docs/porting/` (a README with the workflow, verification and a feature
parity list, plus one DX12 → API guide per backend: the mapping and the hard rules). Lessons 5-7 continue as planned
so the user can judge ports; after them, the rest of the Vulkan backend may be written as a port. Open question to
revisit: compiling the HLSL shaders to SPIR-V with DXC, so GLSL disappears.

## Goal
The samples run on an Android phone, drawn by Vulkan. The work is split in two:
- **Platform (Claude writes it):** build, packaging, driver, input, files, logging, audio, cook. Same shape as the iOS
  and web ports.
- **Vulkan renderer (the user writes it, lesson by lesson):** Claude teaches each step with small examples, then
  reviews the user's code. Claude writes no `rendering/vulkan/` code beyond the Part W skeleton.

Decisions: Android builds from both Mac and Windows; CMake-only packaging (NativeActivity: no Gradle, no Java); GLSL
shaders compiled by `glslc`; learn on Windows first (Vulkan SDK validation, RenderDoc, fast iteration), then move to
the phone. **As simple as possible, and as much as possible in C++.**

## Order
1. **Part W (Claude):** Vulkan client on Windows, renderer skeleton only.
2. **Lessons 0-8 (user) on Windows.**
3. **Part A (Claude):** Android platform; the same renderer builds unchanged for the phone.
4. **Lesson 9 (user) on the phone:** mobile lifecycle; also re-check that lessons 1-8 run there with no validation errors.

## Rule: Vulkan is for Android; Windows is only a learning bench
- Windows Vulkan is a **development-only** build, not a supported platform: DX12 stays the Windows renderer. CMake marks
  it as such, docs mention it in one line, and the samples get no Windows Vulkan preset.
- Windows-only code is limited to `platform/win/win_vulkan_view.*` (surface creation) and one `ME_VULKAN` branch in
  `win_main.cpp`. Everything in `rendering/vulkan/` must compile and behave the same on Android.
- Lessons teach only what Android phones support: no desktop-only extensions or features (geometry shaders, wide lines,
  BC textures, `IMMEDIATE` present mode), `FIFO` present, depth format queried rather than assumed, one graphics queue,
  and memory written for unified (mobile) memory first. Where a desktop habit differs from mobile (tiled GPUs:
  load/store ops, clears), the lesson teaches the mobile way.
- The instance and device are created with only the phone's Vulkan version and features (from its caps), so Windows
  can't hide something the phone lacks.

## Rule: teach the concepts the port uses, in full; skip advanced ones
The renderer draws what `RendererWebGPU`/`RendererMetal` draw today: plain sprites, instanced sets 0 and 1, the UI
sprite and text passes, the debug overlay, a depth buffer, sampled RGBA8 `.dds` textures, `FramesInFlight` frames.
- **Concepts the port uses are taught properly, not narrowed to the one setting the engine picks.** E.g. samplers: the
  filter, address and border options and when each fits, then the engine's choice; blending: the blend equation and
  common modes, then what the sprite passes need; memory: the memory types and when to use each, then the engine's.
- **Concepts the port never uses are left out**, especially advanced ones for advanced games and techniques: compute,
  render-to-texture and post effects, MSAA, mipmapping strategy, multiple queues, bindless and descriptor indexing,
  custom memory allocators, pipeline caches, 3D meshes and lighting, ray tracing.

A topic outside this comes up only if a lesson can't work without it.

## What exists today (found while planning)
- `ME_ANDROID` is already set in the root `CMakeLists.txt`; `moksha/shared/net/socket_platform_posix.cpp` already covers
  Android; `exe_path_posix.cpp` builds there (unused: the driver sets paths).
- `tools/cook/cook.cpp` already parses `android-vulkan` (`CookTarget::AndroidVulkan`) but rejects it as "not supported
  yet".
- Graphics API switches to extend with `ME_VULKAN`: `moksha/shared/math/matrix4.cpp` (3 places, beside `ME_METAL`),
  `moksha/client/app/client_app.{h,cpp}` (includes, `Init`, renderer member), `moksha/client/CMakeLists.txt` (per-API
  source filters, headless filter, entry sources).
- OS switches to extend with `ME_ANDROID`: `client_app.{h,cpp}` (file guard, input forwarding),
  `moksha/client/input/input_manager.{h,cpp}` (friend line and `new InputManager...`), miniaudio's `CMakeLists.txt` (its
  Linux branch links `pthread dl asound m`), `cmake/moksha_game.cmake` (`_moksha_windowed_cook_target`, executable
  shape).
- Every file read goes through `Vfs` (`moksha/shared/file_io/vfs.cpp`, `std::ifstream` on `GetRootPath`); audio loads
  through `Vfs::ReadBytes` + `ma_decoder_init_memory`. So reading APK assets needs only a `Vfs` change.
- `moksha/logging/logging.cpp` writes to `std::cout`, which Android drops.
- Renderer to mirror: `moksha/client/rendering/webgpu/` with `scene/scene_webgpu.*` and `scene/scene_ui_webgpu.*` (the
  simplest; no frames in flight). `rendering/metal/` shows the `FramesInFlight` pattern. Shaders to port to GLSL:
  `resources/shaders/wgsl/` (`sprite_atlas`, `sprite_instanced_atlas`, `ui_sprite`, `ui_text`, plus `structs`,
  `helpers`). Constant layouts shared by all APIs: `rendering/shared/shader_constants.h`.
- Driver templates: `platform/ios/ios_main.mm` (starts at first layout, pause/resume, first finger as the mouse) and
  `platform/win/win_main.cpp` (`CreateWindowExW`, then `app.Init(hWnd)`).
- Earlier port docs with the same pattern: [web_port_plan.md](web_port_plan.md), [web_port_notes.md](web_port_notes.md).

## Design: Android is a layer, like iOS and web
| Layer | New for Android | Counterpart |
|---|---|---|
| Entry and frame driver | `moksha/client/platform/android/android_main.cpp` (NativeActivity + the NDK's `android_native_app_glue`, pure C++) | `platform/ios/` |
| Window access | `VulkanView` (`rendering/vulkan/vulkan_view.h`), implemented per OS in `platform/<os>/` | `MetalView`, `WebGPUView` |
| Input | `input/input_manager_android.*`: first finger as the mouse | `input_manager_ios.*` |
| Renderer | `rendering/vulkan/`, `scene/scene_vulkan.*`, `scene/scene_ui_vulkan.*` (**user**) | `rendering/metal/`, `rendering/webgpu/` |
| Shaders | `resources/shaders/glsl/` → `.spv` (**user**, written during the lessons) | `shaders/metal/`, `shaders/wgsl/` |
| Packaging template | `cmake/android/AndroidManifest.xml.in` | `cmake/ios/Info.plist.in` |

No changes to `Game`, `Scene`, `SceneUI`, UI, physics, animation, `TimeManager`, the network clients or sample game code.

## Part W: Vulkan on Windows, dev-only (Claude)
- `CMakeLists.txt`: renderer `VULKAN` → `ME_VULKAN`, allowed on Android, and on Windows for development only (the Windows
  default stays DX12; `-DMOKSHA_RENDERER=VULKAN` logs a "development only" message). `find_package(Vulkan REQUIRED)` on
  Windows (Vulkan SDK); the NDK's `libvulkan` on Android.
- `moksha/client/CMakeLists.txt`: Vulkan source filter; the other APIs' filters exclude `vulkan`.
- `VulkanView` (`rendering/vulkan/vulkan_view.h`): what the renderer needs from the OS, mainly
  `CreateSurface(VkInstance, VkSurfaceKHR*)` and the instance extensions it requires. Windows implements it in
  `platform/win/win_vulkan_view.*` (`vkCreateWin32SurfaceKHR`), Android later in `platform/android/`.
- `win_main.cpp` / `ClientApp`: an `ME_VULKAN` branch, `Init(const VulkanView*)` instead of `Init(HWND)`.
- `matrix4.cpp`: `ME_VULKAN` beside `ME_METAL` (0..1 depth, column-major). Vulkan's downward Y is handled in the
  renderer with a negative viewport height (lesson 5).
- Cook target `win-vulkan`: `.dds` textures as `win-dx12`, `glsl` → `.spv` via `glslc` (from the Vulkan SDK on Windows,
  the NDK on Android; passed as `--glslc`, like `--dxc`). `moksha_add_game` maps Windows + VULKAN → `win-vulkan`.
  `res_list.json` gets a `glsl` list once the first shader exists.
- `RendererVulkan` skeleton (`InitVulkan(const VulkanView*, const WindowState*)`, empty bodies) so a sample runs with a
  blank window. From here the renderer is the user's.
- A `win-vulkan-dev` preset in the engine's `CMakePresets.json` only.

## Part A: Android platform (Claude). Small steps, each stops for the user to build
**A0. Toolchain (instructions only; Claude installs nothing).** On Mac and Windows: the Android CLI (`android`), then
`android sdk install platform-tools build-tools/<newest stable> platforms/android-<newest stable> ndk/<current LTS>`
(chosen from `android sdk list --all "<pattern>"`), a JDK 17+ (for `apksigner`/`keytool`), Ninja, and a host
`Moksha_Cook` from the normal dev build. A `CMakePresets.json` preset
`android-arm64` (NDK toolchain file, `ANDROID_ABI=arm64-v8a`, `ANDROID_PLATFORM=android-29`, Ninja,
`MOKSHA_COOK_EXECUTABLE`) that reads per-machine paths from env vars (`ANDROID_HOME`, `ANDROID_NDK_HOME`), so one preset
works on both hosts.

**A1. Build config** (on top of Part W).
- `CMakeLists.txt`: `VULKAN` is the default renderer on Android.
- `moksha/client/CMakeLists.txt`: link `vulkan android log`; the Android entry is `platform/android/*.cpp` plus the NDK's
  `android_native_app_glue.c` (compiled into the OBJECT entry library, so no `-u ANativeActivity_onCreate`). The headless filter excludes `input_manager_android`.
- miniaudio's `CMakeLists.txt`: an Android branch (AAudio/OpenSL load at runtime; link only `dl m`).

**A2. Packaging in `cmake/moksha_game.cmake`.** On Android `<target>` is `lib<target>.so` (SHARED). A post-build step,
all CMake custom commands calling SDK tools: `aapt2 link` (manifest from the template, `android.jar`, cooked
`assets/`) → add `lib/arm64-v8a/lib<target>.so` → `zipalign` → `apksigner` with a debug keystore (`keytool` creates it
once in the build folder). Plus `<target>_install` (`adb install -r`, then launch). `DISPLAY_NAME`, `BUNDLE_ID` and
`ORIENTATION` reuse the iOS arguments. Manifest: `android.app.NativeActivity`, `hasCode="false"`, fullscreen theme,
`INTERNET` permission, Vulkan feature. No headless twin and no servers on Android.

**A3. Cook `android-vulkan`:** the same as `win-vulkan`, with `glslc` from the NDK's `shader-tools`.

**A4. Files and logs.**
- `Vfs`: on Android, `Resources` reads go through `AAssetManager` (the APK's `assets/`); `Dlc`/`UserData` stay plain files
  under the app's internal storage. Asset reading lives in `moksha/shared/platform/assets_android.cpp` (OS calls stay in
  `shared/platform/`); `AppPaths` gains the asset manager pointer the driver sets, next to the existing paths.
- `logging.cpp`: under `ME_ANDROID`, write to logcat with `__android_log_write`. This is an OS call outside
  `shared/platform/` (the logging module may depend on nothing), so it is the one exception; note it in the root
  `CLAUDE.md`.

**A5. Driver and input.** `android_main(android_app*)`: poll events without waiting while active, `Tick` once per loop.
On the first `APP_CMD_INIT_WINDOW`: `SetPaths`, `Configure`, `SetWindowMetrics` (window size ÷ density), `Init(view)`.
Focus lost / `APP_CMD_PAUSE` → `Pause`; resume → `Resume`. `APP_CMD_TERM_WINDOW` → tell the renderer the surface is
gone (`ClientApp::SurfaceLost()` / `SurfaceReady(view)`, Android only; the renderer side is lesson 9, and until then the
app stays paused). `false` from `Tick` → `ANativeActivity_finish`. The first touch pointer → `HandlePrimaryTouch`.
`ClientApp` and `input_manager.{h,cpp}` get the `ME_ANDROID` branches the iOS port added.

**A6. Android `VulkanView`** (`platform/android/android_vulkan_view.*`, `vkCreateAndroidSurfaceKHR`). The renderer from
lessons 0-8 then builds for the phone unchanged.

**A7. Samples and docs.** `moksha_samples`: an Android preset, bump the engine pin. Update `moksha/client/CLAUDE.md`,
`tools/CLAUDE.md`, `docs/game_guide.md` and this doc.

## Part B: Vulkan lessons (the user writes the code)
Each lesson: the concept explained in chat with a short example → the exact task (which class, which methods, what
success looks like) → the user writes it → Claude reads and reviews it (correctness, validation, house style) → the user
builds and checks the checkpoint (on Windows for lessons 0-8, on the phone for lesson 9). **Teaching style (the user's
preference since lesson 1):** one function at a time, split into numbered steps; each step names the exact Vulkan
calls and structs with a snippet and explains every field, then the user types it and asks for a review before the next
function. Explain any new term (layer, surface, ...) in plain words when it first appears. Code comments never name a lesson
(they would outlive the lessons); unfinished work is marked `TODO:` with what is missing. One lesson per session is fine;
nothing moves on until the checkpoint passes. Lessons are taught against `RendererDX` (the user wrote it by hand, and
DX12 shares Vulkan's explicit model); classes mirror the DX12 ones. What it draws matches `RendererWebGPU`/`RendererMetal`.

| # | Topic | Writes | Checkpoint |
|---|---|---|---|
| 0 | Vulkan's model mapped onto the engine's existing `RendererDX` classes; handles, `VkResult`, struct chains | — | — |
| 1 | Instance, validation layer, debug messenger; enumerate GPUs; compare with the phone's caps (Vulkan Caps Viewer app) | `RendererVulkan::InitVulkan` (part) | log shows GPU name and Vulkan version |
| 2 | Surface through `VulkanView`, queue family, logical device, queue | renderer | log: device created, no validation errors |
| 3 | Swapchain, image views, render pass and framebuffers, command pools and buffers, fences and semaphores, `FramesInFlight`; clear via the render pass's load op; first RenderDoc capture | renderer | window cleared to a color |
| 4 | GLSL → SPIR-V, shader modules, pipeline layout, graphics pipeline; vertex/index buffers and memory types | `ShaderVulkan`, `PSOVulkan`, `QuadVulkan` | one colored quad |
| 5 | Descriptor sets and pools, uniform buffers per frame in flight, dynamic offsets (as WebGPU); Y-flip viewport | `UploadBufferVulkan` | quad moves with the camera |
| 6 | Images, staging copies, layout transitions and barriers, samplers; load cooked `.dds` | `TextureVulkan` | a textured sprite |
| 7 | Scene sprites: per-sprite data, instanced sets via a storage buffer; depth buffer | `SceneVulkan` | a sample's game scene draws |
| 8 | UI sprite and text passes, debug overlay | `SceneUIVulkan` | full sample, UI and text |
| 9 | **On the phone:** pause, surface lost and recreated, swapchain out of date, pre-rotation, `End`, vsync via present modes | renderer | background/foreground works; clean shutdown |

- Window resize on Windows is out of scope (the engine has none); lesson 9 covers swapchain recreation.
- Vulkan version: **1.1 core, no extensions beyond the surface and swapchain** (decided 2026-11-07). It reaches
  nearly every Vulkan phone, Play filters the rest through the manifest, and the test phone reports 1.1.128. So:
  `VkRenderPass` + one `VkFramebuffer` per swapchain image (no dynamic rendering), `vkCmdPipelineBarrier` (no
  synchronization2), shaders cooked with `--target-env=vulkan1.1`. The instance and device ask for 1.1 on Windows too.
- Test phone limits that shape the design: `maxBoundDescriptorSets` = 4 (the spec minimum; desktops report 32). A
  pass may bind at most 4 sets, so lesson 5 groups bindings by how often they change rather than one set per DX12 table.
- On Android the validation layer is Khronos's prebuilt `.so`, packed into the APK only when
  `MOKSHA_VULKAN_VALIDATION_DIR` is set.

## Verification
- Part W: a sample built with `-DMOKSHA_RENDERER=VULKAN` on Windows opens a blank window; the DX12 build is unchanged.
- Each Part A step: the user builds the `android-arm64` preset on Mac and on Windows; `python tools/check_layers.py`
  passes; desktop builds (DX12, Metal, web, headless) are unaffected.
- After A6: `<target>_install` installs a sample, logcat shows engine logs, resources load from the APK, touch reaches
  the game, audio plays.
- Each lesson: its checkpoint, with no validation-layer messages in the log.
- End: every sample in `moksha_samples` runs on the phone; `git diff --stat` shows no changes outside the files listed.
