# Web port: plan

Status (2026-11-04): phases 0 and 1 done on Emscripten 6.0.13 (the systems check matches the reference under Node, engine
tests 102/102). Phases 2-4 pass in Safari. Phase 5 is built and waiting for the user's test. Research and sources: [web_port_notes.md](web_port_notes.md).

## Goal
Samples playable in a browser from a local server, as a learning hobby project (a website can come later). Keep everything
simple: no compression, performance tuning or other production work. An agent writes the code and builds it on the Mac. The user does the browser testing.

## Design rule: web is a layer, like iOS
The web port adds a platform folder, a graphics backend and a cook target. Game-facing systems don't change.

| Layer | New for web | Counterpart |
|---|---|---|
| Entry and frame driver | `moksha/client/platform/web/` | `platform/ios/` |
| Canvas and device access | `WebGPUView` (`rendering/webgpu/webgpu_view.h`), implemented in `platform/web/` | `MetalView` |
| Input | `moksha/client/input/input_manager_web.*` | `input_manager_ios.*` |
| Renderer | `moksha/client/rendering/webgpu/`, `scene/scene_webgpu.*`, `scene/scene_ui_webgpu.*` | `rendering/metal/`, `scene_metal.*` |
| Shaders | `resources/shaders/wgsl/` | `resources/shaders/metal/` |
| Sockets | `moksha/shared/net/socket_platform_web.cpp` (stub) | `socket_platform_posix.cpp` |
| Page template | `cmake/web/shell.html.in` | `cmake/ios/Info.plist.in` |

### Every change to existing engine code
Anything beyond this list is a design problem to raise with the user first.

| File | Change |
|---|---|
| `moksha/shared/math/matrix4.cpp` | Add `ME_WEBGPU` beside `ME_METAL` in 3 switches. Without it, `Orthographic` returns identity. Metal's conventions apply. |
| `moksha/client/app/client_app.{h,cpp}` | `ME_WEB` in the file guards; `ME_WEBGPU` beside the `Init` and renderer branches; web input forwarding, as iOS has |
| `moksha/client/input/input_manager.{h,cpp}` | `friend` line and `new InputManagerWeb()` |
| `CMakeLists.txt`, `moksha/client/CMakeLists.txt` | `WEBGPU` renderer, source filters, web entry sources |
| `moksha/client/third_party/miniaudio-0.11.23/CMakeLists.txt` | Emscripten branch, since the catch-all links `pthread dl asound m` |
| `cmake/moksha_game.cmake` | `web-webgpu` cook target, `.html` output, Node-run headless twin, no servers |
| `tests/CMakeLists.txt` | Link the engine tests as a Node program on web |
| `tools/cook/cook.cpp` | Accept `web-webgpu`; cook `wgsl` shaders |
| `resources/res_list.json` | List the WGSL shaders |
| Docs | `moksha/client/CLAUDE.md`, `tools/CLAUDE.md`, `docs/game_guide.md`, samples `CLAUDE.md` |

**No changes** to `Game`, `Scene`, `SceneUI`, the UI system, physics, animation, `Vfs`, `TimeManager`, `GameClient`,
`ContentClient`, `AudioSystem` or sample game code. Check at the end with `git diff --stat`.

## Decisions
- **WebGPU only, no WebGL2 fallback.** One backend, matching the Metal design. The page shows a short message when
  WebGPU is missing. A WebGL2 backend could be added later without changing this layer.
- **Single-threaded, no Asyncify, no JSPI.** Works on any static host without special headers.
- **Audio uses miniaudio's default ScriptProcessorNode path.** Its source supports non-threaded Emscripten builds and
  unlocks audio on user input. AudioWorklets need Asyncify, so they stay out. ScriptProcessorNode is deprecated but is
  the simple option. Revisit if audio glitches or a browser removes it.
- **Pinned Emscripten through emsdk.** This Mac uses 6.0.13. WebGPU
  comes from `--use-port=emdawnwebgpu`.
- **Fixed logical size.** The game keeps its configured size (e.g. 450×800), and CSS scales the canvas to fit. The backing
  store is CSS size × device pixel ratio. Input is mapped back to logical coordinates. No resize support in the engine.

## Driver flow (`platform/web/web_main.cpp`)
Async steps run before `ClientApp::Init`, as iOS waits for first layout. State is passed through callback `void*`
arguments, never globals.

1. `main`: `Utils::SetPaths` to `/resources/`, `/dlc/` and `/user_data/`. The `--preload-file` data is already there.
2. Mount IDBFS on `/user_data/` with `autoPersist` and load it with `syncfs(true)`.
3. Request adapter and device, using callbacks. If WebGPU is missing, tell the page to show its fallback.
4. `Configure`, set canvas size, `SetWindowMetrics`, `Init(webGpuView)`.
5. `emscripten_set_main_loop_arg` at the display rate runs `Tick`. `false` from `Tick` only logs.
6. Visibility change: `Pause`/`Resume`.

IDBFS's `autoPersist` copies each write to IndexedDB, so `Vfs` stays unchanged. A save written just before a tab is killed can
be lost. All games on one origin share one database, which is safe because each keeps its saves in its own folder.

## Renderer design
Same passes and order as `RendererMetal`: plain sprites, instanced set 0, instanced set 1, UI, debug UI.

**What gets simpler:** no residency, barriers, fences or `FlushCommandQueue`. `queue.writeBuffer` copies data, so one copy
of each buffer is enough, without `FramesInFlight` copies.

**Bind groups, created at scene build time, never per draw:**
- Group 0: per-pass uniform, atlas uniform (dynamic offset).
- Group 1: texture and nearest clamp sampler, one group per texture.
- Group 2: per-sprite uniform (dynamic offset, 256-byte stride) or the instance storage buffer.

Per-sprite and atlas data go into one shared buffer each, written once per frame, not 1,000 separate buffers.

**Must match Metal:**
- `rgba8unorm-srgb` textures. Canvas in its preferred format, with an sRGB view format and pipelines to match.
- Alpha blend: src alpha / one minus src alpha for colour, one / zero for alpha.
- Depth Less with writes on for sprites, Always with no writes for UI. Clear to 1.0. Depth texture follows the canvas size.
- Counter-clockwise front faces, back culling. Clear colour 0.01.
- 96-byte instance stride. Alpha discard below 0.1. Flipped V and the atlas UV helper.

**WGSL:**
- The cook resolves `#include "structs.wgsl"` and `helpers.wgsl` textually, since WGSL has no includes.
- Uniform arrays use a 16-byte stride, so don't mirror `float padding[3]` as an array. Sprite shaders declare only the
  fields they read.
- Log `uncapturederror` and shader compilation messages through `ME::LogError`.

## Phases
Each phase ends at its gate, and the agent stops there and reports. The next phase starts once the user has tested and
agreed. A failed gate stops the work. Everything uses the `web` preset (live engine link). The agent checks phases 0 and 1
itself. From phase 2 on, the user checks the gates in the browser. The agent may build and run for this task, but the user
makes all commits and pushes.

**0. Setup.** The user installs emsdk. Add a per-machine `web` preset to `CMakeUserPresets.json`, using emsdk's toolchain
file, `MOKSHA_COOK_EXECUTABLE` from `build-live`, tests off, build folder `build-web/`.
*Gate:* `emcc` compiles a test file, and the preset configures.

**1. Engine on wasm, no graphics.** Socket stub, CMake changes, the headless twin as a Node program.
*Gate:* `Sample_SystemsCheck_Headless` runs under emsdk's Node and matches checksum `299C1597` and fixed-point raw `1270944`
(its network check fails, as web has no sockets). The engine's tests pass under Node. This proves the wasm32 build before
any web code exists.

**2. Page, driver, input, audio.** HTML shell, web driver, input manager, miniaudio, and a first `RendererWebGPU` that only
clears and presents.
*Gate:* in Safari, the page loads and clears; the systems check's input and audio checks pass; sound plays after a click;
pausing works when the tab is hidden.

**3. Cook and shaders.** `web-webgpu` target, include handling, four WGSL passes, and creating their pipelines at startup.
*Gate:* cook output is correct, and all pipelines build without errors in Safari's console.

**4. Renderer.** Plain sprites, instanced sets, UI, debug overlay.
*Gate:* Tower Climb, UI Demo, Evolution, Survivors and Graphics Stress run with no console errors, and look the same as
their native Metal builds in layout, colour and depth order.

**5. Saves.** IDBFS mount and sync.
*Gate:* Tower Climb's best height survives a reload.

**6. Publish.** Build the chosen samples and copy each one's files to `dist/<sample>/`, with a small index page.
*Gate:* every sample plays from a local static server. There is no website yet, so nothing is uploaded.

**7. Close out.** Update the `CLAUDE.md` files and docs.
*Gate:* Mac and iOS builds still work; the user confirms one Windows build; `python3 tools/check_layers.py` passes;
`git diff --stat` touches only the listed existing files.

## Samples
- **Publish:** Tower Climb first, then Survivors, Evolution, UI Demo, Graphics Stress.
- **Build but don't publish:** Systems Check (test tool), Rally and Net Test. Browsers have no raw UDP or TCP, so they need
  a separate WebSocket or WebTransport port with TLS on the server. Out of scope.

## Testing
Browser testing is manual (decided 2026-11-03). The agent builds each phase, starts a local static server and lists what
to check. The user opens the page, checks it and reports back, with any console errors. Safari is the test browser. Trying
a phone is optional.

## Install on the Mac
Required:
- Python 3.10 or newer, for emsdk (the system Python 3.9.6 is too old): `brew install python`, then
  `export EMSDK_PYTHON=/opt/homebrew/bin/python3`.
- emsdk, on this Mac in `/Users/amrit/work/gamedev/tools/emsdk/emsdk` (about 2 GB): `git clone https://github.com/emscripten-core/emsdk.git <that path>`, then
  `emsdk install latest` and `emsdk activate latest`, then record the version here. It includes its own
  Node, which runs the phase 1 checks.

Optional:
- Ninja, faster builds: `brew install ninja`.
- Google Chrome, to try a Chromium browser before publishing: `brew install --cask google-chrome`.
- Firefox, manual cross-browser check: `brew install --cask firefox`.

Already installed: Homebrew, CMake 4.4.5, git, Xcode, Safari, and the host `Moksha_Cook` in `build-live/`.
