# Writing a game on Moksha

For anyone (human or AI) writing a game in its own repo against the engine. Engine internals live in the
`CLAUDE.md` files; this page is only what a game needs. Recipes: [game_cookbook.md](game_cookbook.md).

## Hard rules
- C++20. **No exceptions, no RTTI**: no `try`/`catch`/`throw`, `dynamic_cast`, `typeid` (the build rejects
  them). Report failure with `bool`, null pointers or an error enum.
- **Never block the frame**: no endless loops, no sleeping, no waiting on I/O. Do one frame's work in
  `Update`/`FixedUpdate` and return. The engine must not require threads.
- **Files only through `ME::Vfs`** (`moksha/shared/file_io/vfs.h`), with paths relative to a root
  (`Resources`, `Dlc`, `UserData`). No `fopen`, `std::ifstream` or absolute paths.
- **No OS calls** (`windows.h`, POSIX, Cocoa) in game code; the engine covers the platforms.
- Game code lives in the game's own namespace, never in `ME`. Engine headers are included qualified:
  `#include "moksha/client/game/game.h"`.

## Repo shape
```
<game>/CMakeLists.txt         FetchContent the engine, then moksha_add_game(...)
<game>/game_main.cpp          MOKSHA_GAME(MyGame::GameMain, "Title")
<game>/game/, <game>/scene/   Game subclass, Scene and SceneUI subclasses
<game>/resources/res_list.json   assets to cook, paths relative to this file
<game>/resources/config/settings.ini
```
`moksha_add_game(<Target> SOURCES ... RES_LIST resources/res_list.json [HEADLESS])` builds the executable and
cooks the engine's resources plus the game's next to it. `HEADLESS` adds `<Target>_Headless` (no window,
renderer, input or audio; same game code). After adding or removing a `.cpp`, re-run `cmake -B build`.
On iOS the game is an app bundle: optional `DISPLAY_NAME`, `BUNDLE_ID` (default `MOKSHA_BUNDLE_ID_PREFIX` plus the
target) and `ORIENTATION PORTRAIT|LANDSCAPE|ANY` fill its Info.plist; other platforms ignore them.

## Lifecycle
1. `CreateGame` (via `MOKSHA_GAME`) constructs the game before any engine system exists. Do nothing there.
2. `Init(ME::Engine&)`: call `Game::Init(engine)` first, then create **both** `scene` and `uiScene` and call
   `Init()` on each. The engine reads them right after `Init` returns; a null scene crashes.
   Also set up physics here (`engine->physics->SetScene`, `SetCollisionListener(this)`).
3. `Start()`: everything exists, including the renderer and audio.
4. Every frame: N × `FixedUpdate(fixedDt)` (60 Hz by default; timers, physics and animation run in the same
   step) → `Update(dt)` → UI → render. Gameplay that must be deterministic goes in `FixedUpdate`.
5. `End()`: clear your timers, delete what you created (`scene`, `uiScene`, physics scene), then call
   `Game::End()`. `OnPause`/`OnResume` on minimize; on mobile and web the app can die without `End`.

## What `ME::Engine` gives you (`moksha/client/app/engine.h`)
`time` (frame clock), `timers` (delayed/repeating `Delegate` calls), `input` (keys, mouse), `physics` (AABB
collision reports), `animation`, `audio`, `ui`, `gameClient` (UDP to a game server), `contentClient` (DLC
sync), `config` (final settings), `window` (actual size), `Quit()`, `IsHeadless()`. Keep the pointer the
base class stores in `engine`; never put engine systems in globals.

## Scenes
- `ME::Scene` is plain arrays (`ME::Span<T>`: `data` + `count`), not an ECS. `Scene::Init()` calls
  `CreateResources`, `BuildLights`, `BuildCamera`, then the `Build*Transforms`/`Build*Renderers` virtuals.
  Override only what you use and **call the base first**: base `CreateResources` allocates every array.
- Arrays have fixed capacities (`moksha/shared/misc/engine_limits.h`); you set `.count`. `Span::operator[]`
  does not bounds-check, so check against the capacity yourself.
- Drawable today (DX12): sprites (`spriteTransforms`/`spriteRenderers`), the two instanced sprite sets, UI
  sprites and text. `meshRenderers` (3D) are not drawn yet.
- `ME::SceneUI` holds the UI tree (`Panel`, `Label`, `Image`, `Button`), built in `BuildUIElements()`.

## Traps
- **Texture slots are fixed.** `spriteTexturePaths[i]` pairs with `textureAtlasProperties[i]`. Plain sprites
  use their own `textureId`/`textureAtlasPropsId`; instanced set 0 always draws slot 1 and set 1 always
  slot 2, so fill those slots when you use the sets. A `SceneUI` needs the UI atlas at slot 0 and the font at
  slot 1 (copy the cookbook's UI recipe).
- **Instanced sets are sized once**, from `.count` at startup. Allocate the maximum you will need and park
  unused instances off-screen. After changing an instance's transform or atlas index, set its renderer's
  `bDirty = true`, or the change isn't uploaded. Plain sprites are re-read every frame.
- **Collision results are yours to delete**: in a `CollisionCallback` override, `delete result` (or call
  `Game::CollisionCallback`, which does). Physics only reports overlaps; it never moves anything, so update
  collider bounds yourself (`ColliderAABB::UpdateTransform`).
- **Audio ids are array indices** into `scene->sfxPaths` / `scene->musicPaths`, loaded once after `Init`.
  Fill them in `CreateResources`.
- **Coordinates:** world space is Y-up. `spriteCamera` is orthographic: `orthographicSize` is half the view
  height in world units, and its `aspectRatio` defaults to 1.33, so set it from `engine->window`. UI space is
  Y-down from the top-left, in pixels.
- **Debug overlay slots 0-2 are the engine's** (FPS, game name, scene name). Use slots 3-15.
- **Textures are cooked**: list `textures/x.png` in `res_list.json`, load `textures/x.dds` in code. Other
  files go under `copy`. A file missing from the list doesn't exist at runtime.
- `GetKeyDown` = held; `GetKeyPressed` = went down this frame.

## Where to look next
- [game_cookbook.md](game_cookbook.md): minimal game plus copyable snippets per system.
- `moksha_samples` repo: `evolution` (instanced sprites, procedural map, camera), `ui_demo` (UI tree, button),
  `net_test` + `server` (game server, custom verbs, headless bots).
- Engine headers: each starts with a short contract block. Read the header, not the `.cpp`.
