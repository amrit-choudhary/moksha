# Moksha Engine

C++20 game engine (namespace `ME`). This repo holds engine code only. Games, including the
engine's samples (`moksha_samples` repo), live in their own repos and build against the engine.

## Engine vs. game
- Engine code is generic: no API, comment or default is shaped around one game. Game-specific
  behavior goes into the game's repo, usually as a subclass or data the game supplies.
- **Games depend on the engine; the engine never depends on a game.** The engine reaches game
  code only through contracts it defines itself: the game factory (`CreateGame`/`DestroyGame`,
  defined by the game), `Game` virtuals, listener interfaces, `Delegate`s, and data the game
  supplies (scenes, resource lists, settings). Games may use every engine module directly.
- **No game content in the engine:** no game names, game data or demo-specific defaults in
  engine code, comments, assets or tests.
- The samples in `moksha_samples` are the engine's test harness and carry no backward-compat
  weight. If an API change makes the engine more consistent, change it, fix the samples in the
  same session, and bump the samples' engine pin.
- When it isn't clear whether something belongs in the engine or a game, ask.
- Game-facing docs: [docs/game_guide.md](docs/game_guide.md) (rules, lifecycle, traps; games import it into
  their `CLAUDE.md`) and [docs/game_cookbook.md](docs/game_cookbook.md) (recipes). Update both in the same
  change as any game-facing API or behavior, and keep their snippets compiling against the current API.

## Modules and includes
- Engine code lives under `moksha/<module>/`; includes are qualified from the repo root:
  `#include "moksha/shared/math/vec2.h"`. No bare `src/...`, no `../`.
- Allowed dependencies between modules:
  - `logging`, `test_framework` → nothing
  - `shared` → `logging`
  - `server` → `shared`, `logging`
  - `client` → `shared`, `logging`
  - `tools/*` → `shared`, `logging`
  - `tests` → anything

  `shared` never includes `client` or `server`, so servers build without client code.
- A non-qualified include (`"vec3.h"`) is only allowed for a file in the same folder.
- Per-module notes: [client](moksha/client/CLAUDE.md), [shared](moksha/shared/CLAUDE.md),
  [server](moksha/server/CLAUDE.md), [tools](tools/CLAUDE.md). Update them when a module's shape changes.
- `python tools/check_layers.py` checks both rules (stdlib only; exit code 1 lists violations).
  Run it after changing includes.

## Build system
- C++20, CMake 3.31+.
- **RTTI and exceptions are disabled globally** (MSVC: `/GR- /EHs-c-` + `_HAS_EXCEPTIONS=0`,
  C4530 promoted to an error; else: `-fno-rtti -fno-exceptions`). Do not write or suggest code
  using `try`/`catch`/`throw`, `dynamic_cast`, or `typeid`.
- The client builds as two variants from the same sources: `Moksha_Client` (windowed, graphics API from
  `MOKSHA_RENDERER`) and `Moksha_ClientHeadless` (`ME_HEADLESS`: null renderer, input and audio). Each has its
  entry OBJECT library (`Moksha_Entry`, `Moksha_EntryHeadless`). The API define is per target, never global.
- Games build with `moksha_add_game()` (`cmake/moksha_game.cmake`). It owns the per-platform executable
  shape and runs `Moksha_Cook` (`tools/cook`), which cooks `resources/res_list.json` (engine) and then the
  game's list next to the executable. Engine resources stay minimal: shaders, one font, one UI atlas.
- `file(GLOB ...)` source lists are collected at configure time: after adding or removing a
  `.cpp`, re-run `cmake -B build`.

## Platforms
Targets: Windows (DirectX 12, later Vulkan), macOS and iOS (Metal), Android (Vulkan), Linux
(Vulkan client later; servers and headless client), Web (Emscripten, WebGPU/WebGL). Every
change is checked against all of them, even those not built yet:
- **OS and graphics API are separate defines.**
  - OS: `ME_WIN`, `ME_MAC`, `ME_IOS`, `ME_ANDROID`, `ME_LINUX`, `ME_WEB`.
  - Graphics API: `ME_DX12`, `ME_METAL`, `ME_VULKAN`, `ME_WEBGPU`; `ME_HEADLESS` for the
    windowless client.
  - Window, input and entry code switch on the OS define; rendering, scene and math
    conventions switch on the graphics API define.
- **The app loop never blocks.** The engine exposes one frame (`Tick()`); a per-platform
  driver repeats it. On iOS, Android and the web the OS calls the frame, so no engine code
  may loop forever or require the app to exit cleanly.
- **All asset and file access goes through `Vfs`.** Assets live in the APK, the app bundle or
  a virtual filesystem on some platforms; writable data has its own per-platform root.
- **OS calls only in** `moksha/shared/platform/` (one file per OS, for code every program needs),
  `moksha/client/platform/<os>/`, `rendering/<api>/` and `shared/net/socket_platform_*`.
- **The engine must not require threads** (the web needs special builds for them).
- **Window size is a request:** mobile and web decide it; headless keeps it for layout.
- Test host for Linux servers and headless clients: a Raspberry Pi 4B (Linux ARM64).

## Code conventions
Apply to all engine code (not vendored third-party code). Existing code that breaks a rule
isn't rewritten unprompted; new code, and lines edited anyway, follow them.
- **No exceptions or RTTI**. Report failure with return values (`bool`, null pointer, error enum).
  `std::filesystem` calls must use the `std::error_code` overloads.
- **`class`, never `struct`**, even for plain-data aggregates; use an explicit `public:` label.
- **No `std::vector` in new code**: prefer caller-owned memory (pointer + capacity), fixed-size
  arrays, or `Span<T>`, always with bounds checks. If growth is truly needed, ask first.
- **No singletons / static instance pointers** to give callbacks or systems access to object
  state. Pass explicit bound context (the `Delegate` pattern: `void* object` + function pointer,
  or a listener interface). If a global looks like the only option, raise it as a design problem.
- **Comments: max 2 lines**, stating the engine-level contract. Never justify code by what one
  game or demo currently does.
- Headers include what they use directly (e.g. `<cstdint>`): MSVC/Apple clang hide missing
  includes, g++ (Linux builds) does not.
- Avoid names that are `windows.h` macros in new APIs (`GetFreeSpace`, `PeekMessage`,
  `SendMessage`, `DeleteFile`, `MoveFile`, `CreateDirectory`, ...).
- Format with the repo's `.clang-format` (Google-based, 4-space indent, 120 columns).

House style (follow the surrounding code):
- Each header starts with a short `/** ... */` block stating the contract, then `#pragma once`,
  standard includes, then project includes.
- Nested namespaces per module (`ME::Net`, `ME::Time`).
- Explicit `Init()` / `End()` lifecycles. Copy/move are deleted when a class owns resources.
- A `//` comment of at most 2 lines above each public method. Members get in-class defaults
  (`= 0`, `= nullptr`).
- `out`-prefixed out-params, `enum class X : uint8_t` results with a trailing comment per
  value, handles where 0 means none, `Delegate` for callbacks.
- Naming per `.clang-tidy`: CamelCase types and functions; camelBack members, parameters and
  locals.

## Working with AI assistants
- Never `git commit` or stage; the user reviews and commits manually.
- Don't build or run anything to verify; the user does that. Ask before running anything.
  Running clang-format on touched files is fine.
- For new systems: small, independently buildable steps, simplest first, explain each, then stop.
- The user works on 3 machines (2 Windows, 1 Mac) synced through git: put project knowledge in
  `CLAUDE.md` files, not local memory.

## Dependencies
Vendored, treat as opaque (don't modify or deep-dive): cJSON, miniaudio, metal-cpp,
stb_perlin.
