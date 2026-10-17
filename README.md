# Moksha Game Engine

A C++20 game engine with native renderer backends, built from scratch with minimal
external dependencies. This repository holds the engine only; every game, including the
engine's [samples](https://github.com/amrit-choudhary/moksha_samples), lives in its own
repository and builds against the engine.

## Features
- **Rendering**: DirectX 12 (Windows) and Metal (Apple) backends, plus a headless backend for
  windowless clients. A data-oriented scene (struct-of-arrays, not ECS) is consumed by the
  renderer each frame.
- **App loop**: the engine owns the entry point and the frame; a game provides a factory and
  callbacks. Fixed-step simulation with variable-rate update and rendering, and a timer system
  for scheduled one-shot and repeating calls.
- **Physics**: AABB colliders and a physics scene with collision callbacks.
- **Animation**: data-driven sprite animation clips and texture atlases (JSON).
- **Audio**: audio system with a miniaudio backend and a no-op fallback.
- **Input**: platform input managers, with a null input for headless clients.
- **UI**: a small retained-mode layout engine, plus an on-screen debug overlay.
- **Networking**: cross-platform UDP and TCP sockets (Winsock / POSIX), length-prefixed
  message framing, a binary game protocol, a generic game-server module, and a content
  protocol with file manifests for delivering game data without shipping a new client.
- **Core**: vector/matrix math, 24.8 fixed-point numbers for deterministic simulation, an
  xoshiro128** random number generator, fixed-capacity data structures (`Grid`,
  `RingBuffer`, `Pool`, `Span`), a virtual file system, INI/CSV/JSON/DDS parsers, and
  bounds-checked binary serialization.
- **Tooling**: an asset cook tool, a content server, a minimal logging library and a
  hand-rolled unit test framework.

## Platforms
| Target            | Graphics API         | Status |
|-------------------|----------------------|--------|
| Windows           | DirectX 12 (Vulkan)  | supported |
| macOS             | Metal                | in progress: being moved onto the current app loop; UI and atlas rendering to follow |
| iOS               | Metal                | planned |
| Android           | Vulkan               | planned |
| Web (Emscripten)  | WebGPU / WebGL       | planned |
| Linux             | Vulkan               | planned client; servers and headless client supported (incl. Raspberry Pi, ARM64) |

## Design principles
- No exceptions and no RTTI; failures are reported through return values.
- Caller-owned, fixed-capacity memory over growing containers.
- No singletons; systems receive explicit context.
- A general engine first: no API is shaped around a single game.
- Games depend on the engine; the engine never depends on a game.

## Building the engine
C++20 and CMake 3.31 or newer. On Windows the cook also needs
[dxc](https://github.com/microsoft/DirectXShaderCompiler) and
[texconv](https://github.com/microsoft/DirectXTex) on `PATH`.

```
cmake --preset dev
cmake --build --preset dev
build/tests/Debug/Moksha_Tests        # engine unit tests
python tools/check_layers.py          # module dependency check
```

Built on its own, the engine builds its libraries, tools (`Moksha_Cook`, `Moksha_ContentServer`) and
tests. Games build it as a dependency; then only the libraries and the cook are built.

| Option | Default | Meaning |
|---|---|---|
| `MOKSHA_RENDERER` | `DX12` on Windows, `METAL` on Apple, `NONE` elsewhere | graphics API of the windowed client; `NONE` builds the headless client only |
| `MOKSHA_BUILD_CLIENT` | `ON` | client libraries (servers don't need them) |
| `MOKSHA_BUILD_TESTS` / `MOKSHA_BUILD_TOOLS` | `ON` when top-level | engine tests / content server |
| `MOKSHA_COOK_EXECUTABLE` | empty | a host-built cook, for cross-compiled builds |

## Starting a new game
A game is its own repository. It fetches the engine at a pinned commit and, while you work on both,
builds your local engine clone instead.

```
my_game/
  CMakeLists.txt            project, engine pin, add_subdirectory(client) [+ server]
  CMakePresets.json         pinned preset (committed)
  CMakeUserPresets.json     live preset (gitignored, per machine)
  my_game.code-workspace    the game and the engine in one VS Code window
  .claude/settings.json     lets Claude Code read and edit the engine clone
  CLAUDE.md  README.md  .gitignore  .clang-format  .clang-tidy
  client/
    CMakeLists.txt          moksha_add_game(...)
    game_main.cpp           MOKSHA_GAME(MyGame, "Title") or CreateGame/DestroyGame
    game/                   class MyGame : public ME::Game
    scene/                  class SceneMyGame : public ME::Scene, class SceneUIMyGame : public ME::SceneUI
    resources/              res_list.json, config/settings.ini, the game's own assets
  server/                   optional: moksha_add_server(...), an IGameServerListener
  shared/                   optional: code for client and server, e.g. the game's message verbs
```
Keep the engine clone at `<Projects>/moksha/moksha` and the game at `<Projects>/my_game/my_game`;
the paths below assume that. Includes are qualified from the game's root (`"client/game/my_game.h"`),
engine headers as `"moksha/client/..."`. Game code goes in its own namespace (here `MG`), not `ME`.

**`CMakeLists.txt`**
```cmake
cmake_minimum_required(VERSION 3.31)
project(MyGame LANGUAGES CXX C)

# The engine builds without exceptions; drop MSVC's default /EHsc so it doesn't clash with /EHs-c- (D9025).
if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# The engine commit this game builds against. Bump it after pushing the engine changes the game needs.
set(MOKSHA_PIN <full commit sha>)

include(FetchContent)
FetchContent_Declare(moksha
    GIT_REPOSITORY https://github.com/amrit-choudhary/moksha.git
    GIT_TAG ${MOKSHA_PIN})
FetchContent_MakeAvailable(moksha)

add_subdirectory(client)
```

**`client/CMakeLists.txt`**
```cmake
moksha_add_game(MyGame_Client
    SOURCES
        game_main.cpp
        game/my_game.cpp
        scene/scene_my_game.cpp
    RES_LIST resources/res_list.json
    # LIBRARIES <the game's own libraries>
    # HEADLESS   adds MyGame_Client_Headless, a windowless console build
)
```
`moksha_add_game` owns the platform details (Windows GUI exe, later app bundles and so on), so a game
never branches on the platform. Link the game's own libraries through `LIBRARIES`, not
`target_link_libraries`: on a platform without a windowed client the main target doesn't exist.

**`client/game_main.cpp`**
```cpp
#include "client/game/my_game.h"
#include "moksha/client/game/game_factory.h"

MOKSHA_GAME(MG::MyGame, "My Game")
```
For more control (reading command-line arguments, changing the default window), define
`ME::Game* ME::CreateGame(const ME::AppArgs& args, ME::AppConfig& config)` and
`void ME::DestroyGame(ME::Game* game)` instead.

**The game class**: in `Init(ME::Engine& engine)` call the base `Init`, then create `scene` and
`uiScene` (both are required) and call their `Init()`. Engine systems are reached through `engine->`
(`input`, `timers`, `physics`, `audio`, `gameClient`, ...), and `engine->Quit()` ends the game. Delete
the scenes in `End()`.

**`client/resources/res_list.json`** lists only the game's files; the engine's shaders, font and UI atlas
are added automatically (paths relative to this file):
```json
{
    "copy": ["config/settings.ini", "texture_data/my_atlas.json"],
    "textures": ["textures/my_atlas.png"]
}
```

**`client/resources/config/settings.ini`**
```ini
[window]
title=My Game
width=1280
height=720
vsync=1

[time]
fixedStepFPS=60

[net]
serverIP=127.0.0.1
serverPort=9310
```
Every key can be overridden on the command line: `MyGame_Client.exe --width=1920 --serverIP=10.0.0.5`.

**`CMakePresets.json`**
```json
{
    "version": 6,
    "configurePresets": [ { "name": "pinned", "binaryDir": "${sourceDir}/build" } ],
    "buildPresets": [ { "name": "pinned", "configurePreset": "pinned" } ]
}
```

**`CMakeUserPresets.json`** (add it to `.gitignore`)
```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "live",
            "binaryDir": "${sourceDir}/build-live",
            "cacheVariables": { "FETCHCONTENT_SOURCE_DIR_MOKSHA": "${sourceDir}/../../moksha/moksha" }
        }
    ],
    "buildPresets": [ { "name": "live", "configurePreset": "live" } ]
}
```

**`my_game.code-workspace`**
```json
{
    "folders": [ { "name": "my_game", "path": "." }, { "name": "moksha (engine)", "path": "../../moksha/moksha" } ],
    "settings": {
        "files.exclude": { "**/_deps": true },
        "search.exclude": { "**/build": true, "**/build-*": true },
        "cmake.sourceDirectory": "${workspaceFolder:my_game}"
    }
}
```

**`.claude/settings.json`** and the first line of the game's **`CLAUDE.md`**
```json
{ "permissions": { "additionalDirectories": ["../../moksha/moksha"] } }
```
```
@../../moksha/moksha/CLAUDE.md
```

### Daily workflow
- Work with the `live` preset: the game builds your engine clone, so engine edits show up on the next
  build and one debugger session covers both. Configuring warns when `MOKSHA_PIN` is behind the clone.
- Commit and push the engine first, then set `MOKSHA_PIN` to that commit and push the game.
- `pinned` builds exactly the pinned commit from GitHub; use it on build machines, servers and fresh clones.
- A game server: `moksha_add_server(MyGame_Server SOURCES main.cpp ... RES_LIST resources/res_list.json)`,
  whose `main` is `MyListener listener; ME::ServerApp app; return app.Run(argc, argv, listener);`. On a
  server-only machine configure
  with `-DMOKSHA_BUILD_CLIENT=OFF`.

## Known issues
- **macOS**: the Mac client still uses the previous app API and doesn't compile; Metal draws no UI, text or
  atlas sprites yet.
- **Untested at runtime**: mesh, audio and animation loading have no sample using them; the mesh and audio
  loaders were moved onto `Vfs` without a run.
- **Rendering**: the DX12 UI passes use fixed texture slots (sprite atlas at 0, font at 1).
- **Server**: clients are never forgotten (no disconnect or timeout), so a server accepts at most 255
  connects in its lifetime; no SIGTERM handling; every datagram is logged.
- **Client loop**: network messages are received after rendering, which adds up to a frame of latency.
- **Code**: `DebugSystem` is reached through a static instance; `InputManager` keeps key state in statics.

## Third-party code
Vendored libraries, each under its own license:
- [cJSON](https://github.com/DaveGamble/cJSON): JSON parsing.
- [miniaudio](https://miniaud.io/): audio playback.
- [metal-cpp](https://developer.apple.com/metal/cpp/): Metal C++ headers.
- [stb_perlin](https://github.com/nothings/stb): Perlin noise.

## License
MIT. See [LICENSE](LICENSE).
