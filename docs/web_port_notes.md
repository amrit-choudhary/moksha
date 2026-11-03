# Web port: research notes

Status (2026-11-03): research behind [web_port_plan.md](web_port_plan.md), which holds the decisions and the work order.
Where the two differ, the plan wins. Nothing is built or tested for the web yet.

## Goal
Playable samples on a personal website: compile to WebAssembly with Emscripten, draw on a `<canvas>`, host static files
(`.wasm`, `.js`, `.data`, `.html`) and embed each game in an `<iframe>`. Any static HTTPS host works.

## Leaning: WebGPU, single-threaded
- WebGPU matches the Metal backend's model and conventions (0..1 depth, top-left texture origin). The instanced shaders
  read a storage buffer, which WebGPU has and WebGL2 lacks.
- Cost: browsers without WebGPU can't play, so the page needs a video or screenshot fallback. Support: Chrome/Edge,
  Safari 26, recent Firefox (Windows, Apple silicon Mac); Firefox on Android and Linux appears to lag (unconfirmed).
- No threads means no COOP/COEP headers. Godot's web export also defaults to single-threaded.

## How WebGPU compares to Metal
Same structure as classic Metal, closer to that than to Metal 4. Going away: residency, barriers, most frame
synchronisation, keeping resources alive for the GPU. Different or new:
- Bind groups are objects created ahead of time, the biggest design difference from Metal 4's argument tables.
- WGSL has no `#include`, so the cook must join `structs` and `helpers` into each pass.
- `packed_float3` has no equivalent and `vec3` aligns to 16 bytes, so the light structs need care.
- Getting the device is async. Validation errors arrive asynchronously, not at the failing call.
- Data goes through `queue.writeBuffer`, an extra copy (about 10 MB per frame in Graphics Stress).
- A canvas can't use an sRGB format directly: configure `bgra8unorm` with an sRGB view format.
- Dynamic uniform offsets align to 256 bytes. Viewports had to fit inside the framebuffer as of 2023 (affects letterboxing).

Hand-writing it: device setup, clear and present, and the first sprite pass are worth doing by hand. The remaining passes
are repetition. Vulkan (planned for Android and Linux) would teach more.

## What other engines do
- Godot 4: WebGL2 only, single-threaded by default since 4.3.
- Bevy: WebGL2 default, WebGPU opt-in (old source, around 0.10).
- sokol: both backends. Its 2023 tests: Windows Chrome about 64k uniform-update draws on WebGL2 vs 16k on WebGPU at 60 Hz;
  M1 Mac 8.5k vs 11k. It needed a bind group cache.
- Defold: WebGPU selectable (forum post of unclear date).
- pmtech (solo C++ engine, WebGL, about 6 years old): about 20 working days; threads and WebGL were the main trouble.
- No detailed write-up found of a small custom engine shipping on WebGPU.

Takeaways: use `--use-port=emdawnwebgpu` with Emscripten 4.0.10+ (`-sUSE_WEBGPU` is deprecated); create bind groups at
startup rather than per draw; avoid Asyncify.

## Port work other than the renderer
Findings from reading the code on 2026-11-03.

**Must change**
- `moksha/client/app/client_app.{h,cpp}`: the whole of both files is guarded to Windows, Mac, iOS and headless, so `ME_WEB`
  needs adding, plus its input and init branches.
- Driver: new `moksha/client/platform/web/`, running `Tick` from `emscripten_set_main_loop`. Set paths and window
  metrics first, as iOS does. `Tick` returning false only logs. `Shutdown` can't be relied on.
- `moksha/client/CMakeLists.txt` has no web entry and stops with a fatal error.
- `moksha/client/third_party/miniaudio-0.11.23/CMakeLists.txt`: the catch-all `else()` links `pthread dl asound m`, so
  Emscripten needs its own branch.
- `moksha/shared/net/socket_platform_posix.cpp` excludes `ME_WEB`, so a web socket file is needed or the link fails. A
  stub that always fails is enough for offline samples.
- `moksha/client/input/`: new web input manager from HTML5 keyboard, mouse and touch events. `InputManager` needs its
  `friend` line. The iOS manager's "first finger is the mouse" rule can be reused.
- `cmake/moksha_game.cmake`: web cook target and `.html` output; skip servers and headless twins, as iOS does.
- Link flags: `--preload-file` for resources, `-sALLOW_MEMORY_GROWTH`, possibly a larger `-sSTACK_SIZE`.

**Should work as is, once tested**
- `Vfs`: standard file calls work on Emscripten's in-memory filesystem, with resources preloaded before `main`.
- Time: `TimeManager` uses `std::chrono` and clamps long frames, so a hidden tab shouldn't cause a big catch-up.
- Audio loading: sounds decode from memory through `Vfs`. miniaudio has an Emscripten backend.
- `ContentClient`: a failed connection already leaves it in `Failed` and the game carries on.

**Needs care**
- Saves: `Vfs::WriteBytes` writes to memory on the web, lost on reload. Mount `user_data/` on IDBFS and sync after writes.
- Audio: browsers start audio only after a user gesture.
- `GameClient::Init` returns false without a socket and `ClientApp` ignores the result. That's safe: `Update` returns early
  when the socket isn't open.
- Display: canvas CSS size vs. device pixel ratio, high-refresh monitors, `preventDefault` on game keys, iframe focus.
- wasm32: `size_t` and pointers are 4 bytes, and Emscripten's default stack is small (64 KB by default in recent versions;
  confirm). A quick search found no huge stack arrays, but it wasn't exhaustive.
- Download size: compress with Brotli or gzip; tune with `-O3` or `-Os`, `-flto`.

**Out of scope for now**
Rally and Net Test: browsers have no raw UDP or TCP. They would need WebSocket or WebTransport, with TLS on the server,
since an HTTPS page can't reach an unencrypted one.

## Sources
- sokol WebGPU backend: https://floooh.github.io/2023/10/16/sokol-webgpu.html
- Godot web export: https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_web.html
- Bevy WebGPU: https://bevy.org/news/bevy-webgpu/
- Defold forum: https://forum.defold.com/t/webgpu-and-runtime-import/79825/2
- pmtech port: https://polymonster.co.uk/blog/porting-to-wasm-with-emscripten
- WebGPU browser support: https://web.dev/blog/webgpu-supported-major-browsers
- Emscripten WebGPU: https://emscripten.org/docs/porting/multimedia_and_graphics/WebGPU-support.html
- `-sUSE_WEBGPU` deprecation: https://github.com/emscripten-core/emscripten/commit/72bcff54099548284e6b099c4f0d33128df4e269
- Dear ImGui Emscripten requirement: https://flutter.googlesource.com/third_party/imgui.git/+/83fa0ae2634fad2b8303f5929ba049650bf94bc0%5E%21/
- wasm_webgpu (Asyncify note): https://www.github.com/juj/wasm_webgpu
