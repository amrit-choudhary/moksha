# moksha/client/

Client engine (`Moksha_Client`, `Moksha_ClientHeadless`). Project rules: [root CLAUDE.md](../../CLAUDE.md).

## App, entry and loop
- The engine owns `main` and the loop; a game provides `CreateGame(const AppArgs&, AppConfig&)` /
  `DestroyGame(Game*)` (`game/game_factory.h`, or the `MOKSHA_GAME(GameClass, "Title")` macro).
- `app/client_app.*` (`ClientApp`, Windows, macOS and headless): `Configure(args)` → `Init` → `Tick()` per frame →
  `Shutdown()`. It never loops itself. Drivers per platform: `platform/win/win_main.cpp` (window, message pump,
  `Tick`; minimized → `Pause` and a blocking wait), `platform/mac/mac_main.mm` (the same shape: `NSWindow`, event
  pump with `nextEventMatchingMask`, `Tick`; Cmd+Q and closing end the loop, so `Shutdown` always runs),
  `platform/headless/headless_main.cpp` (plain `main`, `Tick` + `Time::WaitForNextFixedStep`). Keep the Mac
  driver parallel to the Windows one.
- macOS today: a plain executable with `resources/` beside it, input, miniaudio, and `RendererMetal` on Metal 4
  (`MTL4CommandQueue`, one allocator and command buffer, the layer's residency set, one frame in flight flushed
  with a shared event like DX12's `FlushCommandQueue`), which only clears and presents. The content view is an
  `MTKView` subclass (`MEView`, no delegate) the loop draws with `[view draw]`; it hands out a drawable only inside
  draw, so `Tick` runs from `MEView`'s `drawRect:`. Waiting for the drawable paces the loop. The renderer reaches the view only through
  `rendering/metal/metal_view.h`. `metal_cpp_impl.cpp` compiles metal-cpp's implementation once.
- The other `rendering/metal/` files and `scene_metal` predate both the current `Scene` API and Metal 4 (device-made
  pipelines, per-frame command buffers); they are out of the build and get rewritten on Metal 4, mirroring their
  DX12 counterparts, as they're ported. Mac work left, in order: scene data on the GPU (`scene_metal`), sprites
  and the atlas sprite sets, the DX12 UI passes (`scene_ui_dx`, UI sprites/text, debug overlay), Retina
  (`WindowState::dpiScale`) and resizing; offline-compiled shaders (a `.metallib` from the cook, needs Xcode's
  Metal toolchain) instead of compiling at startup; an `.app` bundle when distribution needs one.
- Metal shaders (`resources/shaders/metal/`) are self-contained `.metal` files (no `#include`: they compile from
  source at startup with `MTL4Compiler`), each mirroring its `.hlsl` and repeating the structs and atlas helper it
  uses. The `mac-metal` cook copies them and turns textures into the same RGBA8 `.dds` as `win-dx12`.
- Mac known issues (deferred): keys pressed with Cmd stick (AppKit swallows their key-up; GLFW forwards Cmd key-ups
  with a local event monitor); releasing one Shift/Ctrl/Option/Cmd while the other side is held reads as still down
  (decide press/release from the key's own state, as GLFW does); held keys stick when the window loses focus; a
  fully covered window stalls about 1 s per frame in the drawable wait (watch the occlusion state and skip drawing;
  undecided whether the game keeps running or pauses then).
- Config layering (`app/app_config.*`, later wins): engine defaults → `CreateGame` → `resources/config/settings.ini`
  (`[window] title width height vsync fullscreen`, `[time] fixedStepFPS`, `[net] serverIP serverPort
  contentServerIP contentServerPort`) → command line `--key=value` with the same keys. Window size is a request.
- `app/engine.h` (`Engine`): what the game gets in `Game::Init(Engine&)` — `time`, `timers`, `input`,
  `physics`, `animation`, `audio`, `ui`, `gameClient`, `contentClient`, `config`, `window`, plus `Quit()` and
  `IsHeadless()`. Passed in, never global.
- `Tick()` order: `BeginFrame` → input → N × (timers, `FixedUpdate`, physics, animation) at the fixed step →
  `Update`, UI, debug overlay → input post → render (skipped when paused) → game client, content client, audio.
- `Game` lifecycle: `Init(Engine&)` (create `scene` and `uiScene`; both are required) → `Start` → `FixedUpdate` /
  `Update` → `End`; `OnPause`/`OnResume` on minimize (mobile backgrounding later).

## Scene and rendering (not ECS)
- `scene/scene.h` (`Scene`) and `scene/scene_ui.h` (`SceneUI`) are plain struct-of-arrays data (transforms,
  sprite/mesh/text renderers, colliders, lights, cameras) filled by game subclasses through `Build*` virtuals.
  No graphics-API calls in them.
- Backends read the scenes each frame: DX12 (`rendering/directx/`, `scene/scene_dx.*`, `scene/scene_ui_dx.*`),
  Metal (`rendering/metal/`, `scene/scene_metal.*`), null (`rendering/null/`, headless: updates scenes, draws
  nothing). They switch on `ME_DX12` / `ME_METAL` / `ME_HEADLESS`, never on the OS define.
- The DX12 UI passes use fixed texture slots: a `SceneUI`'s sprite atlas at index 0 and font at index 1.
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
  own pass; it is reached through a static instance (known issue).
- `anim/`: data-driven sprite animation clips (JSON). `utils/json_utils.*`: texture atlas and animation JSON.
- `world/`: tile data; `TileMap` is an empty stub.

## Network clients
- `net/game_client.*` (`GameClient`): UDP client for a game server (`[net] serverIP/serverPort`). Engine verbs
  (connect, ping, chat, data) are handled here; game verbs (`GameProtocol::FIRST_GAME_VERB` and up) go to the
  game's `IGameClientListener` (`SetListener`), and the game sends them with `SendGameMessage`.
- `net/content_client.*` (`ContentClient`): syncs `dlc/` next to the exe with a content server in the
  background at startup, one state-machine step per frame; on failure the game continues with what it has.

## Files
All file reads go through `Vfs` (`moksha/shared/file_io/vfs.h`) with paths relative to a root
(`Resources`, `Dlc`, `UserData`). Never build paths from `GetResourcesPath()` or open files directly.

## Vendored
`third_party/` (miniaudio, metal-cpp): opaque, don't modify. `metal/metal-cpp` is Apple's
`release/metal-cpp_macOS27.0.1_iOS27.0.1` (github.com/apple/metal-cpp); update it by replacing the folder. No
metal-cpp-extensions: the renderer reaches the MTKView only through `rendering/metal/metal_view.h`, which
`platform/mac/mac_main.mm` implements in Objective-C++.
