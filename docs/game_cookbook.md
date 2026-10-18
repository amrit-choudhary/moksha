# Moksha game cookbook

Copyable recipes for games. Rules and traps: [game_guide.md](game_guide.md). Snippets use a game namespace
`MyGame`; class and member names are placeholders.

## Minimal game: one sprite moved with the arrow keys
Uses the engine's UI atlas as a placeholder sprite, so it needs no art of its own.

`CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.31)
project(MyGame LANGUAGES CXX C)
if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")  # engine builds with /EHs-c-
endif()
include(FetchContent)
FetchContent_Declare(moksha GIT_REPOSITORY https://github.com/amrit-choudhary/moksha.git GIT_TAG <commit>)
FetchContent_MakeAvailable(moksha)

moksha_add_game(MyGame
    SOURCES game_main.cpp game/game_main_class.cpp scene/scene_main.cpp
    RES_LIST resources/res_list.json
    HEADLESS)
```

`resources/res_list.json`
```json
{ "copy": ["config/settings.ini"] }
```

`resources/config/settings.ini`
```ini
[window]
title=My Game
width=1280
height=720
```

`game_main.cpp`
```cpp
#include "game/game_main_class.h"
#include "moksha/client/game/game_factory.h"

MOKSHA_GAME(MyGame::GameMain, "My Game")
```

`scene/scene_main.h`
```cpp
/**
 * The game's world: one sprite.
 */
#pragma once

#include "moksha/client/scene/scene.h"

namespace MyGame {

class SceneMain : public ME::Scene {
   public:
    void CreateResources() override;
    void BuildCamera() override;
    void BuildSpriteTransforms() override;
    void BuildSpriteRenderers() override;

    ME::Transform& GetPlayer() { return spriteTransforms[0]; }
};

}  // namespace MyGame
```

`scene/scene_main.cpp`
```cpp
#include "scene/scene_main.h"

#include "moksha/client/utils/json_utils.h"

void MyGame::SceneMain::CreateResources() {
    ME::Scene::CreateResources();  // allocates the arrays; always first
    spriteTexturePaths[0] = "textures/ui/ui_atlas.dds";
    spriteTexturePaths.count = 1;
    ME::JsonUtils::LoadTextureAtlasProps("texture_data/atlas_ui.json", textureAtlasProperties[0]);
    textureAtlasProperties.count = 1;
}

void MyGame::SceneMain::BuildCamera() {
    ME::Scene::BuildCamera();
    spriteCamera->orthographicSize = 100.0f;  // view is 200 world units tall
}

void MyGame::SceneMain::BuildSpriteTransforms() {
    spriteTransforms[0].SetPosition(0.0f, 0.0f, 0.0f);
    spriteTransforms[0].SetScale(16.0f, 16.0f);
    spriteTransforms.count = 1;
}

void MyGame::SceneMain::BuildSpriteRenderers() {
    // quadId, materialId, textureId (slot), atlasPropsId (slot), atlasIndex (tile in the atlas)
    spriteRenderers[0] = ME::SpriteRenderer(0, 0, 0, 0, 399);
    spriteRenderers.count = 1;
}
```

`game/game_main_class.h`
```cpp
/**
 * The game: owns the scenes and moves the player.
 */
#pragma once

#include "moksha/client/game/game.h"
#include "scene/scene_main.h"

namespace MyGame {

class GameMain : public ME::Game {
   public:
    void Init(ME::Engine& inEngine) override;
    void Update(double deltaTime) override;
    void End() override;

   private:
    SceneMain* mainScene = nullptr;
};

}  // namespace MyGame
```

`game/game_main_class.cpp`
```cpp
#include "game/game_main_class.h"

#include "moksha/client/app/app_config.h"
#include "moksha/client/input/input_manager.h"

void MyGame::GameMain::Init(ME::Engine& inEngine) {
    ME::Game::Init(inEngine);  // always first

    mainScene = new SceneMain();
    mainScene->Init();
    mainScene->spriteCamera->aspectRatio =
        static_cast<float>(engine->window->width) / static_cast<float>(engine->window->height);
    scene = mainScene;

    uiScene = new ME::SceneUI();  // required even when empty
    uiScene->Init();
}

void MyGame::GameMain::Update(double deltaTime) {
    ME::Game::Update(deltaTime);
    const float speed = 80.0f * static_cast<float>(deltaTime);
    ME::Vec3 pos = mainScene->GetPlayer().GetPosition();
    if (engine->input->GetKeyDown(ME::Input::KeyCode::LeftArrow)) pos.x -= speed;
    if (engine->input->GetKeyDown(ME::Input::KeyCode::RightArrow)) pos.x += speed;
    if (engine->input->GetKeyDown(ME::Input::KeyCode::UpArrow)) pos.y += speed;
    if (engine->input->GetKeyDown(ME::Input::KeyCode::DownArrow)) pos.y -= speed;
    mainScene->GetPlayer().SetPosition(pos);
    if (engine->input->GetKeyPressed(ME::Input::KeyCode::Escape)) engine->Quit();
}

void MyGame::GameMain::End() {
    delete scene;
    scene = nullptr;
    delete uiScene;
    uiScene = nullptr;
    ME::Game::End();
}
```

## Own art: a texture atlas
1. Put `textures/chars.png` (a grid of equal tiles) under `resources/` and list it under `"textures"`.
2. Add `texture_data/atlas_chars.json` under `"copy"`, same fields as the engine's `atlas_ui.json`:
   `tileSizeX`, `tileSizeY`, `padding`, `numTextures`, `numTilesX`, `numTilesY`, `width`, `height`,
   `paddingType`.
3. In `CreateResources`: `spriteTexturePaths[n] = "textures/chars.dds";` and
   `ME::JsonUtils::LoadTextureAtlasProps("texture_data/atlas_chars.json", textureAtlasProperties[n]);`
4. `atlasIndex` counts tiles row by row from the first tile.

## Many sprites: instanced sets (tiles, crowds, bullets)
```cpp
// CreateResources: set 0 draws slot 1, set 1 draws slot 2. Fill the slots below them too.
spriteTexturePaths[1] = "textures/tiles.dds";
ME::JsonUtils::LoadTextureAtlasProps("texture_data/atlas_tiles.json", textureAtlasProperties[1]);
// ... slot 0 as well, then .count = 2 on both arrays.

// BuildInstancedSpriteTransforms / BuildInstancedSpriteRenderers: size for the maximum, once.
instancedSpriteTransforms0.count = maxTiles;
instancedSpriteRenderers0.count = maxTiles;
for (size_t i = 0; i < maxTiles; ++i) {
    instancedSpriteTransforms0[i].SetPosition(x, y, 1.0f);  // larger z draws behind
    instancedSpriteTransforms0[i].SetScale(tileSize, tileSize);
    instancedSpriteRenderers0[i] = ME::SpriteRenderer(0, 0, 1, 1, tileIndex);
}

// Later, after moving instance i or changing its atlasIndex:
instancedSpriteRenderers0[i].bDirty = true;
// "Removing" an instance: move it far off-screen and mark it dirty.
```

## Sprite animation
```cpp
// clips/walk.json under "copy": {"bLooping": true, "textureAtlasIndex": 0, "spriteCount": 4,
//                                "spriteIndices": [10, 11, 12, 13]}
ME::SpriteAnimClip* walk = nullptr;
if (ME::JsonUtils::LoadSpriteAnimClipFromJSON("clips/walk.json", &walk)) {
    ME::SpriteRenderer& renderer = spriteRenderers[0];
    renderer.animator = new ME::SpriteAnimator(&renderer, 12);  // you own the animator; delete it in End
    renderer.animator->AddClip(walk);                           // the animator owns the clip
}
// Switch clips: renderer.animator->ChangeClip(1);
```
Only animators on the game's `Scene` sprite renderers (plain and instanced) are ticked.

## Input
```cpp
const ME::Input::InputManager* input = engine->input;
input->GetKeyDown(ME::Input::KeyCode::Space);       // held
input->GetKeyPressed(ME::Input::KeyCode::Space);    // pressed this frame
input->GetKeyReleased(ME::Input::KeyCode::Space);   // released this frame
input->GetMouseButtonPressed(ME::Input::MouseButton::Left);
ME::Vec2i mouseWorld = input->GetMouseWorldPos();   // pixels, bottom-left origin, Y-up
ME::Vec2i mouseUI = input->GetMousePos();           // pixels, top-left origin, Y-down
```
Headless builds have no input; everything reads as up.

## UI: labels and a button
```cpp
// SceneUI subclass. The UI atlas at slot 0 and the font at slot 1 are required.
void MyGame::SceneUIMain::CreateResources() {
    ME::SceneUI::CreateResources();
    spriteTexturePaths[0] = "textures/ui/ui_atlas.dds";
    spriteTexturePaths[1] = "textures/font/ascii_ibm_transparent.dds";
    spriteTexturePaths.count = 2;
    ME::JsonUtils::LoadTextureAtlasProps("texture_data/atlas_ui.json", textureAtlasProperties[0]);
    ME::JsonUtils::LoadTextureAtlasProps("texture_data/font_atlas_01.json", textureAtlasProperties[1]);
    textureAtlasProperties.count = 2;
}

void MyGame::SceneUIMain::BuildUIElements() {
    ME::SceneUI::BuildUIElements();
    // text, quadId, textureId (font slot 1), materialId, color, glyph height, glyph width,
    // letterSpacing, lineGap, charsPerLine, alignment
    scoreLabel = new ME::Label("Score: 0", 0, 1, 0, ME::Color::White(), 24, 24, 0, 2, 40,
                               ME::TextAlignment::Left);
    scoreLabel->SetAnchor(ME::UIAnchor::TopLeft);
    scoreLabel->SetPivot(ME::UIPivot::TopLeft);
    scoreLabel->SetOffset(ME::Vec2{20.0f, 20.0f});
    scoreLabel->SetSize(ME::Vec2{300.0f, 40.0f});
    scoreLabel->Init();
    AddUIElement(scoreLabel);

    playButton = new ME::Button();
    playButton->SetAnchor(ME::UIAnchor::Center);
    playButton->SetPivot(ME::UIPivot::Center);
    playButton->SetSize(ME::Vec2{200.0f, 50.0f});
    playButton->Init();
    AddUIElement(playButton);
    // Visuals: a Panel (atlas tile) and a Label with SetParent(playButton), then playButton->SetPanel(panel).
}
// The SceneUI deletes nothing it didn't allocate: delete your elements in its destructor.

// In the Game, after uiScene->Init():
ME::Delegate::Bind<GameMain, &GameMain::OnPlayClicked>(sceneUIMain->GetPlayButton()->onClick, this);
// Update text any time: scoreLabel->SetText(buffer);
```
Full working tree: `ui_demo/scene/scene_ui_demo.cpp` in `moksha_samples`.

## Audio
```cpp
// Scene::CreateResources (after the base call); files listed under "copy" in res_list.json.
sfxPaths[0] = "audio/jump.wav";
sfxPaths.count = 1;
musicPaths[0] = "audio/theme.mp3";
musicPaths.count = 1;

// Anywhere from Start on:
engine->audio->PlayAudio(0, false);  // sfx id = index in sfxPaths
engine->audio->PlayMusic(0, true);
```

## Timers
```cpp
// Member: ME::Time::TimerHandle spawnTimer;
spawnTimer = engine->timers->SetInterval(ME::Delegate::Make<GameMain, &GameMain::SpawnEnemy>(this), 2.0);
engine->timers->SetTimeout(ME::Delegate::Make<GameMain, &GameMain::EndRound>(this), 30.0);
// In End(), before anything the callbacks touch is deleted:
engine->timers->Clear(spawnTimer);
```
Callbacks take no arguments; keep the state they need in members.

## Collision
```cpp
// Scene: fill dynamicColliders / staticColliders (capacity in engine_limits.h).
dynamicColliders[0] = ME::ColliderAABB(1, true, false, ME::PhysicsLayer::Player,
                                       ME::PhysicsLayer::Enemy | ME::PhysicsLayer::Environment,
                                       spriteTransforms[0]);
dynamicColliders.count = 1;

// Game::Init, after scene->Init():
physicsScene = new ME::PhysicsScene();
physicsScene->Init(scene->staticColliders, scene->dynamicColliders);
engine->physics->SetScene(physicsScene);
engine->physics->SetCollisionListener(this);

// Every step after moving the sprite:
scene->dynamicColliders[0].UpdateTransform(scene->spriteTransforms[0]);

// Override; a is the dynamic collider.
void MyGame::GameMain::CollisionCallback(ME::ColliderAABB* a, ME::ColliderAABB* b,
                                         ME::CollisionResultAABB* result) {
    // result->seperation pushes a out of b; result->normal is the hit side.
    delete result;
}
```

## Save and load
```cpp
#include "moksha/shared/file_io/vfs.h"
#include "moksha/shared/serialization/byte_reader.h"
#include "moksha/shared/serialization/byte_writer.h"

uint8_t buffer[256];
ME::ByteWriter writer(buffer, sizeof(buffer));
writer.WriteU32(highScore);
ME::Vfs::WriteBytes(ME::FileRoot::UserData, "save/progress.bin", buffer, writer.GetSize());

size_t size = 0;
if (ME::Vfs::ReadBytes(ME::FileRoot::UserData, "save/progress.bin", buffer, sizeof(buffer), size)) {
    ME::ByteReader reader(buffer, size);
    reader.ReadU32(highScore);  // every Read returns false past the end
}
```
Text data (JSON, INI, CSV) shipped with the game: `ME::Vfs::ReadText(ME::FileRoot::Resources, path, out)`,
then parse with cJSON or the engine's INI/CSV parsers.

## Randomness
```cpp
#include "moksha/shared/random/random_engine.h"

ME::Random rng(12345u);            // fixed seed: same sequence every run (replays, tests)
ME::Random rngTimed("loot", true); // seeded from the name plus the clock
uint32_t roll = rng.NextRange(1, 6);  // inclusive
double t = rng.NextDouble();          // [0, 1)
```
Keep one `ME::Random` per purpose as a member; don't re-create it every frame.

## Debug text and logs
```cpp
ME::DebugSystem::ScreenPrintSlot(3, "Enemies: 12");          // persistent row; slots 3-15 are yours
ME::DebugSystem::ScreenPrint("Level up!", 2.0f);             // fades after 2 s
ME::Log("Loaded level ", levelIndex);                        // stdout; also LogWarning, LogError
```

## Multiplayer messages
```cpp
// Shared verb ids, used by client and server: FIRST_GAME_VERB (0x80) and up.
constexpr uint8_t VERB_MOVE = ME::Net::GameProtocol::FIRST_GAME_VERB + 0;

// Client send (only while engine->gameClient->IsConnected()):
uint8_t payload[16];
ME::ByteWriter writer(payload, sizeof(payload));
writer.WriteU16(x);
writer.WriteU16(y);
engine->gameClient->SendGameMessage(VERB_MOVE, payload, writer.GetSize());

// Client receive: implement ME::IGameClientListener::OnGameMessage, then
engine->gameClient->SetListener(this);
```
Server side (`moksha_add_server`, `IGameServerListener`): see `net_test` and `server/` in `moksha_samples`.

## Startup settings
`resources/config/settings.ini` keys: `[window] title width height vsync fullscreen`, `[time] fixedStepFPS`,
`[net] serverIP serverPort contentServerIP contentServerPort`. The same keys work on the command line
(`--width=800`). Read the result from `engine->config`.
