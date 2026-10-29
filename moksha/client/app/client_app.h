#if defined(ME_WIN) || defined(ME_MAC) || defined(ME_IOS) || defined(ME_HEADLESS)

/**
 * The client app: owns every engine system and the game, and runs one frame per Tick. A per-platform driver
 * (platform/win, platform/mac, platform/ios, platform/headless) creates the window, if any, and repeats Tick; this
 * class never loops by itself.
 */

#pragma once

#include <cstdint>

#include "moksha/client/anim/animation_system.h"
#include "moksha/client/app/app_config.h"
#include "moksha/client/app/engine.h"
#include "moksha/client/audio/audio_system.h"
#include "moksha/client/debug/debug_system.h"
#include "moksha/client/game/game.h"
#include "moksha/client/game/game_factory.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/net/content_client.h"
#include "moksha/client/net/game_client.h"
#include "moksha/client/ui/ui_system.h"
#include "moksha/shared/physics/physics_system.h"
#include "moksha/shared/time/time_manager.h"
#include "moksha/shared/time/timer_manager.h"

#if defined(ME_WIN) && !defined(ME_HEADLESS)
#include "moksha/client/input/input_manager_win.h"
#elif defined(ME_MAC) && !defined(ME_HEADLESS)
#include "moksha/client/input/input_manager_mac.h"
#endif
#if defined(ME_DX12)
#include "moksha/client/rendering/directx/renderer_dx.h"
#elif defined(ME_METAL)
#include "moksha/client/rendering/metal/metal_view.h"
#include "moksha/client/rendering/metal/renderer_metal.h"
#else
#include "moksha/client/rendering/null/renderer_null.h"
#endif

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

#if defined(ME_DX12)
    // Initializes every system for hWnd (whose client area must match the config's size) and starts the game.
    bool Init(HWND hWnd);
#elif defined(ME_METAL)
    // Initializes every system for view (the window's content view) and starts the game. Tick must run inside the
    // view's draw callback, where the view hands out the frame's drawable.
    bool Init(const MetalView* view);
#else
    // Initializes every system without a window and starts the game. The config's size is kept for layout.
    bool Init();
#endif

    // Runs one frame. False once the game called engine->Quit(); the window should then close.
    bool Tick();

    // Ends the game and every system. Safe to call more than once and after a failed Init.
    void Shutdown();

    // The window went to the background or came back; the game is told through OnPause/OnResume.
    void Pause();
    void Resume();

    // The app's clock, for drivers that pace the loop themselves.
    const Time::TimeManager& GetTime() const;

#if defined(ME_WIN) && !defined(ME_HEADLESS)
    // Feeds a Windows input message to the input system.
    void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(ME_MAC) && !defined(ME_HEADLESS)
    // Feed macOS input events to the input system: a virtual key code, a position in view points (top-left
    // origin), and a button number (0 left, 1 right, 2 middle, 3-4 extra).
    void HandleKeyEvent(uint16_t keyCode, bool isDown);
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(int button, bool isDown);
#endif

   private:
    Game* game = nullptr;
    Engine engine;
    AppConfig config;
    WindowState window;

    Time::TimeManager timeManager;
    Time::TimerManager timerManager;
    Input::InputManager inputManager;
    GameClient gameClient;
    ContentClient contentClient;
    PhysicsSystem physicsSystem;
    AnimationSystem animationSystem;
    AudioSystem audioSystem;
    UISystem uiSystem;
    DebugSystem debugSystem;
#if defined(ME_WIN) && !defined(ME_HEADLESS)
    Input::InputManagerWin* winInputManager = nullptr;
#elif defined(ME_MAC) && !defined(ME_HEADLESS)
    Input::InputManagerMac* macInputManager = nullptr;
#endif
#if defined(ME_DX12)
    RendererDX renderer;
#elif defined(ME_METAL)
    RendererMetal renderer;
#else
    RendererNull renderer;
#endif

    bool initialized = false;
    bool paused = false;
    // Shutdown can run from both the entry and the destructor; shut networking down only once.
    bool networkingStarted = false;
};

}  // namespace ME

#endif  // ME_WIN || ME_MAC || ME_IOS || ME_HEADLESS
