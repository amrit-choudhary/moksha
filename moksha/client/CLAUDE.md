# moksha/client/

Client engine (`Moksha_Client`, `Moksha_ClientHeadless`). Project rules: [root CLAUDE.md](../../CLAUDE.md).

## App, entry and loop
- The engine owns `main` and the loop; a game provides `CreateGame(const AppArgs&, AppConfig&)` /
  `DestroyGame(Game*)` (`game/game_factory.h`, or the `MOKSHA_GAME(GameClass, "Title")` macro).
- `app/client_app.*` (`ClientApp`, Windows, macOS, iOS, web and headless): `Configure(args)` → `Init` → `Tick()` per frame →
  `Shutdown()`. It never loops itself. Drivers per platform: `platform/win/win_main.cpp` (window, message pump,
  `Tick`; minimized → `Pause` and a blocking wait), `platform/mac/mac_main.mm` (the same shape: `NSWindow`, event
  pump with `nextEventMatchingMask`, `Tick`; minimized or fully covered → `Pause` and a blocking wait, woken by a
  posted empty event when uncovered; Cmd+Q and closing end the loop, so `Shutdown` always runs),
  `platform/headless/headless_main.cpp` (plain `main`, `Tick` + `Time::WaitForNextFixedStep`). Keep the Mac
  driver parallel to the Windows one, and at feature parity with the iOS one: a feature added to one Apple driver
  goes into the other in the same change, unless the other OS has no counterpart.
- macOS today: a plain executable with `resources/` beside it, input, miniaudio, and `RendererMetal` on Metal 4,
  mirroring `RendererDX` class for class: `SceneMetal` (`SceneDX`), `UploadBufferMetal`, `TextureMetal`,
  `QuadMetal`, `PSOMetal` (pipelines through `MTL4Compiler`, plus depth states), `ShaderMetal` (loads the cooked
  `.metallib` through `Vfs`). No root signature or descriptor heap: one argument table at the fixed indices in
  `MetalBindings` (`pso_metal.h`, Metal snapshots it at each draw) and one residency set holding every buffer and
  texture, added to the queue with the layer's set. Depth is a renderer-owned memoryless `Depth32Float` texture sized
  to the drawable. `Constants::FramesInFlight` frames in flight: one command allocator per frame, one copy of each
  `UploadBufferMetal`'s data per frame, and a shared event that frees each slot. Metal 4 doesn't keep resources alive
  for the GPU, so call `FlushCommandQueue` before freeing anything a frame used. Draws every pass
  RendererDX draws: plain sprites, both instanced sets, the UI sprite and text passes and the debug overlay
  (`SceneUIMetal`); the C++ instance data structs carry explicit padding to 96 bytes, the stride of a `float4x4` struct
  in Metal shaders. The content view is
  an `MTKView` subclass (`MEView`, no delegate) the loop draws with `[view draw]`; it hands out a drawable only
  inside draw, so `Tick` runs from `MEView`'s `drawRect:`. Waiting for the drawable paces the loop. The renderer
  reaches the view only through `rendering/metal/metal_view.h`, which each OS implements over its own view
  (`platform/mac/mac_metal_view.*`); vsync goes through it too, since `displaySyncEnabled` is macOS-only.
  `metal_cpp_impl.cpp` compiles metal-cpp's implementation once. Run with `MTL_DEBUG_LAYER=1` for API validation.
- Constant buffer layouts shared by both APIs are in `rendering/shared/shader_constants.h`. Mac work left, in order:
  resizing (window metrics are set once, before `Init`, as on iOS); an `.app` bundle when distribution needs one, and with it the iOS
  paths (resources from `NSBundle`, `user_data/` and `dlc/` in Application Support); the plain dev executable keeps
  them next to itself, as on Windows.
- Metal shaders (`resources/shaders/metal/`) mirror the hlsl ones file for file: each pass's `.metal` includes
  `structs.metal` and `helpers.metal` (the counterparts of `structs.hlsl`, `helpers.hlsl`). The `mac-metal` cook
  compiles each listed shader to a `.metallib` (Xcode's Metal compiler, macOS 26 minimum) and turns textures into the
  same RGBA8 `.dds` as `win-dx12`; nothing compiles at startup.
- DX12 known issue: `SceneDX::MakeSpriteInstanceBuffer` returns early when instanced set 0 is empty, so a scene using
  only set 1 gets no buffer for it (`SceneMetal` creates each set independently).
- Mac known issues (deferred): keys pressed with Cmd stick (AppKit swallows their key-up; GLFW forwards Cmd key-ups
  with a local event monitor); releasing one Shift/Ctrl/Option/Cmd while the other side is held reads as still down
  (decide press/release from the key's own state, as GLFW does); held keys stick when the window loses focus.
- iOS (`platform/ios/`, fully separate from `platform/mac/`, ARC): `ios_main.mm` runs `UIApplicationMain` with one
  window scene (`MEAppDelegate`, `MESceneDelegate`, named in `cmake/ios/Info.plist.in`). `MEViewController` owns the
  `ClientApp` and starts it at the first layout: `Utils::SetPaths` (resources in the bundle's `data/`, `user_data/` and
  `dlc/` in Application Support, DLC out of backups), `SetWindowMetrics` (points and `contentScaleFactor`), then
  `Init(IosMetalView)`. Its `MEView` (an MTKView on its own 60 Hz timer) calls `Tick` from `drawRect:`; resign/become
  active pause the view and call `Pause`/`Resume`; a `false` from `Tick` only logs. The first finger is the mouse
  (`InputManagerIos`). iOS kills apps without notice, so never rely on `Shutdown`. Device only: the Simulator has no
  Metal 4. miniaudio builds as Objective-C there (`miniaudio_ios.m`).
- Web (`platform/web/`, cross-compiled with Emscripten; single-threaded, no Asyncify): `web_main.cpp` loads saved data
  (IDBFS on `/user_data/` with `autoPersist`, so `Vfs` is unchanged), then asks for an adapter and device, all by callback,
  and starts the `ClientApp` in the last callback: `Configure`, canvas size, `SetWindowMetrics`, `Init(WebGPUView)`, then
  `emscripten_set_main_loop_arg` runs `Tick` at the display's rate. The game keeps its configured window size, and the page
  shrinks the canvas to fit once at startup (no resizing), so input is scaled back to window units. Tab visibility calls
  `Pause`/`Resume`; a `false` from `Tick` only logs. Input: `InputManagerWeb`, keys by DOM `KeyboardEvent.code`, the first
  finger as the mouse; losing focus releases everything. Resources are packed into the page's `.data` and preloaded to
  `/resources/`. Audio is miniaudio's default ScriptProcessorNode path, which unlocks on the first input. WebGPU needs a
  secure page (`localhost` or HTTPS); the page shows a message without it.
- Config layering (`app/app_config.*`, later wins): engine defaults → `CreateGame` → `resources/config/settings.ini`
  (`[window] title width height vsync fullscreen`, `[time] fixedStepFPS`, `[net] serverIP serverPort
  contentServerIP contentServerPort`, `[debug] debugOverlay`, `[ui] safeAreaTop safeAreaBottom safeAreaLeft safeAreaRight`, copied into `WindowState` for the
  UI layout) → command line `--key=value` with the same keys. Window size is a request.
- `app/engine.h` (`Engine`): what the game gets in `Game::Init(Engine&)` — `time`, `timers`, `input`,
  `physics`, `animation`, `audio`, `ui`, `gameClient`, `contentClient`, `config`, `window`, plus `Quit()` and
  `IsHeadless()`. Passed in, never global.
- `Tick()` order: `BeginFrame` → input → N × (timers, `FixedUpdate`, physics, animation) at the fixed step →
  `Update`, UI, debug overlay → input post → render (skipped when paused) → game client, content client, audio.
- `Game` lifecycle: `Init(Engine&)` (create `scene` and `uiScene`; both are required) → `Start` → `FixedUpdate` /
  `Update` → `End`; `OnPause`/`OnResume` on minimize (macOS also while fully covered; iOS on leaving the
  foreground).

## Scene and rendering (not ECS)
- `scene/scene.h` (`Scene`) and `scene/scene_ui.h` (`SceneUI`) are plain struct-of-arrays data (transforms,
  sprite/mesh/text renderers, colliders, lights, cameras) filled by game subclasses through `Build*` virtuals.
  No graphics-API calls in them.
- `PlatformRenderer` (`rendering/shared/renderer.h`) is what each backend implements. `ClientApp` holds its backend by
  concrete type and creates it with the backend's own `InitDX`/`InitMetal`/`InitWebGPU`/`InitVulkan`/`InitNull`; the
  rest goes through the interface.
- Backends read the scenes each frame: DX12 (`rendering/directx/`, `scene/scene_dx.*`, `scene/scene_ui_dx.*`),
  Metal (`rendering/metal/`, `scene/scene_metal.*`), WebGPU (`rendering/webgpu/`, `scene/scene_webgpu.*`,
  `scene/scene_ui_webgpu.*`), Vulkan (`rendering/vulkan/`, Android; in progress, see
  [docs/android_port_plan.md](../../docs/android_port_plan.md)), null (`rendering/null/`, headless: updates scenes, draws
  nothing). They switch on `ME_DX12` / `ME_METAL` / `ME_WEBGPU` / `ME_VULKAN` / `ME_HEADLESS`, never on the OS define.
- **Backends stay in step.** A structural improvement made in one backend (buffer layout, ownership, lifecycle, failure
  handling; not an API-specific detail) belongs in every other backend too, but is never applied right away: Claude
  adds it to the backport backlog (Android: the "Backport list" in
  [docs/android_port_plan.md](../../docs/android_port_plan.md)), reminds the user of the open items at the end of each
  step, and applies one only when the user asks.
- **2D pipelines cull no faces** on every backend (counter-clockwise stays the front face), so a sprite mirrored by a
  negative scale still draws; only DX12's unused 3D pipeline culls back faces.
- **Porting a rendering feature** (DX12 first, then the others): [docs/porting/](../../docs/porting/README.md) has
  the workflow, verification, the feature parity table (update it with each feature) and one DX12 → API guide per
  backend. Read the backend's guide before porting to it.
- Vulkan reaches the window only through `VulkanView` (`rendering/vulkan/vulkan_view.h`: instance extensions,
  `CreateSurface`, pixel size), implemented per OS in `platform/<os>/`. On Windows (`win_vulkan_view.*`) it is a
  development-only build (`-DMOKSHA_RENDERER=VULKAN`, needs the Vulkan SDK); DX12 stays the Windows renderer.
- `RendererVulkan` (in progress; the user writes it lesson by lesson, see the plan) targets Vulkan 1.1 core with only the
  surface and swapchain extensions, and only what phones support. Its classes mirror the DX12 ones: `ShaderVulkan`
  (`shaders/glsl/<name>.vert/.frag.spv` through `Vfs`), `QuadVulkan`, `PSOVulkan` and `PipelineLayoutVulkan` (static
  factories like `PSODirectX` / `RootSigDx`; the renderer owns and destroys what they return), `SamplerVulkan` (named
  static factories, like DX12's static samplers), `UploadBufferVulkan`, `TextureVulkan` and `utils_vulkan.*`
  (`FindMemoryType`, `CreateBuffer` / `DestroyBuffer`, one-time upload commands). One graphics + present queue family,
  `EXCLUSIVE` sharing. Buffers use CPU-written memory, shared GPU-fast memory first (phones), plain host-visible memory
  as the fallback; textures are device-local, filled through a staging copy. Descriptor sets: set 0 per pass, set 1
  per sprite (dynamic uniform buffers, one copy per frame picked by dynamic offset), set 2 texture (combined image
  sampler; the sampler is immutable, built into the set layout, and destroyed after the layouts). Set 1 holds the
  sprite constants (binding 0) and their atlas properties (binding 1); dynamic offsets go set by set, binding by binding.
  Depth: one image shared by all frames, format queried (D32, X8_D24, then D16), a transient attachment in lazily
  allocated memory when the GPU has it, cleared by the load op and never stored.
  A `VkRenderPass` with one framebuffer per swapchain image clears through its load op; `FramesInFlight` command pools,
  fences and image-available semaphores, render-finished semaphores per swapchain image; viewport and scissor are
  dynamic, Y flipped by a negative viewport height. `SceneVulkan` (`scene/scene_vulkan.*`, made by `SetScenes` after a
  device wait) owns a scene's sprite textures, one buffer for all sprites, one for all atlases, and its own pool and
  sets (set 2 per texture, a set 1 per non-empty instanced set). `Draw` binds set 0 once, kept across both atlas
  pipeline layouts (same set 0 layout); `DrawSprites` binds sets 1 and 2 per sprite, skipping out-of-range ids.
  Instanced sets read a storage buffer (`UploadBufferVulkan::Kind::Storage`: elements packed at the std430 stride,
  only each frame's copy aligned, the descriptor range the whole array; set 1 binding 0) by `gl_InstanceIndex`,
  texture and atlas slots 1 and 2 as in every backend, each set made on its own.
  The scene pipelines blend straight alpha (`SRC_ALPHA`/`ONE_MINUS_SRC_ALPHA`, alpha `ONE`/`ZERO`) with depth on,
  as DX12. `SceneUIVulkan` (`scene/scene_ui_vulkan.*`, one for the game's UI and one for the debug overlay, each
  rebuilt after a device wait by `SetScenes` / `SetDebugUIScene`) owns a UI scene's textures (atlas slot 0, font
  slot 1), two pass buffers (packed window size; the GLSL block declares the `uint` directly, as a std140 struct
  would round up past the 4-byte C++ constants), the atlases, and the UI sprite and text instance buffers at
  capacity, of which each frame copies (`CopyElements`) and draws only the live counts. The UI pipelines
  (`CreatePSO2DUISprite` / `CreatePSO2DUIText`, with their own `CreateLayout2DUISprite` / `CreateLayout2DUIText`,
  as DX12's UI PSOs and root signatures) have depth off; `DrawUIScene` draws the sprite and text passes after the
  scene, each binding sets 0-2. Not done yet: swapchain recreation.
  `End()` waits for the device, then destroys in reverse creation order.
- WebGPU (through Emscripten's `emdawnwebgpu` port): `RendererWebGPU` mirrors `RendererMetal` pass for pass, and reaches
  the canvas through `WebGPUView`, which the driver fills in once it has a device. The browser presents and keeps resources
  alive, and `queue.writeBuffer` copies, so there are no frames in flight, fences or `FlushCommandQueue`; never call
  `wgpuSurfacePresent` (it aborts). Bind groups are made once, with scenes, from the layouts in `PSOWebGPU` (`pso_webgpu.h`).
  Per-sprite and atlas constants each share one buffer, read at 256-byte dynamic offsets. The canvas is configured with its
  preferred format and drawn through an sRGB view. Depth state is baked into pipelines. WGSL (`resources/shaders/wgsl/`)
  mirrors the other languages file for file; it declares only the fields it reads, as uniform arrays have a 16-byte stride.
  Errors arrive asynchronously through the device's callback and are logged with a `WebGPU:` prefix.
- DX12 and Metal run `Constants::FramesInFlight` frames in flight: `Draw` ends by waiting for the next frame's slot, and
  each upload buffer holds one copy of its data per frame (on DX12, with one descriptor per frame, at heap index +
  frame). Textures are shared. Call `FlushCommandQueue` before freeing anything a frame used.
- The UI passes (DX12 and Metal) use fixed texture slots: a `SceneUI`'s sprite atlas at index 0 and font at index 1.
- **Metal code uses the Metal 4 API only** (`MTL4CommandQueue`, command allocators, argument tables, residency sets,
  explicit barriers, `MTL4Compiler`), never the older per-encoder binding style (`setVertexBytes`, `setFragmentTexture`).
  Its model matches DX12's, so Metal classes mirror the DX12 ones. Requires Apple silicon (M1+) and macOS/iOS 26+.
- Shaders are loaded cooked (`shaders/hlsl/<name>_vs.cso` / `_ps.cso`), textures as `.dds`; see
  [tools/CLAUDE.md](../../tools/CLAUDE.md) for the cook.

## Other systems
- Same strategy pattern everywhere: `audio/` (`IAudioImpl`: miniaudio, or `audio_impl_dummy.h` headless),
  `input/` (`PlatformInputManager`: win, mac, or the no-op base when headless). Key state in `InputManager`
  is static (existing code; mouse position queries need the instance).
- `ui/`: small retained-mode layout (`UILayoutEngine`, `UIElement` tree: panel, label, image, button) feeding
  `SceneUI` arrays. `debug/`: on-screen overlay (`DebugSystem::ScreenPrintSlot` / `ScreenPrint`) drawn in its
  own pass; it is reached through a static instance (known issue). `AppConfig::debugOverlay` (default: on, off when
  `NDEBUG` is defined) turns it off: it isn't created and its print calls do nothing.
- `anim/`: data-driven sprite animation clips (JSON). `utils/json_utils.*`: texture atlas and animation JSON.
- `world/`: tile data; `TileMap` is an empty stub.

## Network clients
- `net/game_client.*` (`GameClient`): UDP client for a game server (`[net] serverIP/serverPort`). Engine verbs
  (connect, ping, chat, data) are handled here; game verbs (`GameProtocol::FIRST_GAME_VERB` and up) go to the
  game's `IGameClientListener` (`SetListener`), and the game sends them with `SendGameMessage`.
  `SendConnectRequest` sends one datagram and nothing is resent, so the game repeats it until `IsConnected()`
  (the net test sample: every second). No reconnect after the app is backgrounded yet.
- iOS: the first send to a LAN address shows the Local Network prompt (text from `NSLocalNetworkUsageDescription` in
  `cmake/ios/Info.plist.in`); sends to the LAN fail until the user allows it, so the first CONNECT is usually lost.
- `net/content_client.*` (`ContentClient`): syncs `dlc/` next to the exe with a content server in the
  background at startup, one state-machine step per frame; on failure the game continues with what it has.

## Files
All file reads go through `Vfs` (`moksha/shared/file_io/vfs.h`) with paths relative to a root
(`Resources`, `Dlc`, `UserData`). Never build paths from `GetResourcesPath()` or open files directly.

## Vendored
`third_party/` (miniaudio, metal-cpp): opaque, don't modify. `metal/metal-cpp` is Apple's
`release/metal-cpp_macOS27.0.1_iOS27.0.1` (github.com/apple/metal-cpp); update it by replacing the folder. No
metal-cpp-extensions: the renderer reaches the MTKView only through `rendering/metal/metal_view.h`, which
`platform/mac/mac_metal_view.mm` implements in Objective-C++.
