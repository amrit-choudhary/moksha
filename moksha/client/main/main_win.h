#ifdef ME_WIN

/**
 * The client app on Windows: owns every engine system and the game, and runs one frame per Tick. The Windows
 * entry (platform/win/win_main.cpp) creates the window and repeats Tick; this class never loops by itself.
 */

#pragma once

#include "moksha/client/anim/animation_system.h"
#include "moksha/client/app/app_config.h"
#include "moksha/client/app/engine.h"
#include "moksha/client/audio/audio_system.h"
#include "moksha/client/debug/debug_system.h"
#include "moksha/client/game/game.h"
#include "moksha/client/game/game_factory.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/input/input_manager_win.h"
#include "moksha/client/net/content_client.h"
#include "moksha/client/net/game_client.h"
#include "moksha/client/rendering/directx/renderer_dx.h"
#include "moksha/client/ui/ui_system.h"
#include "moksha/shared/physics/physics_system.h"
#include "moksha/shared/time/time_manager.h"
#include "moksha/shared/time/timer_manager.h"

namespace ME {

class ClientApp {
   public:
    ClientApp();
    ~ClientApp();

    ClientApp(const ClientApp&) = delete;
    ClientApp& operator=(const ClientApp&) = delete;
    ClientApp(ClientApp&&) = delete;
    ClientApp& operator=(ClientApp&&) = delete;

    // Builds the final config: engine defaults, then the game's CreateGame, then settings.ini, then the command
    // line. Call once, before the window exists; the window is created from the returned config.
    const AppConfig& Configure(const AppArgs& args);

    // Initializes every system for hWnd (whose client area must match the config's size) and starts the game.
    bool Init(HWND hWnd);

    // Runs one frame. False once the game called engine->Quit(); the window should then close.
    bool Tick();

    // Ends the game and every system. Safe to call more than once and after a failed Init.
    void Shutdown();

    // The window went to the background or came back; the game is told through OnPause/OnResume.
    void Pause();
    void Resume();

    // Feeds a Windows input message to the input system.
    void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    Game* game = nullptr;
    Engine engine;
    AppConfig config;
    WindowState window;

    Time::TimeManager timeManager;
    Time::TimerManager timerManager;
    Input::InputManager inputManager;
    Input::InputManagerWin* winInputManager = nullptr;
    GameClient gameClient;
    ContentClient contentClient;
    PhysicsSystem physicsSystem;
    AnimationSystem animationSystem;
    AudioSystem audioSystem;
    UISystem uiSystem;
    DebugSystem debugSystem;
    RendererDX renderer;

    bool initialized = false;
    bool paused = false;
    // Shutdown can run from both the entry and the destructor; shut networking down only once.
    bool networkingStarted = false;
};

}  // namespace ME

#endif  // ME_WIN
