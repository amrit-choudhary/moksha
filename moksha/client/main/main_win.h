#ifdef ME_WIN

/**
 * Game Engine side Main Entry Point for Windows.
 * Since Windows entry is a bit different, we have a simplified main entry point.
 */

#pragma once

#include <cassert>

#include "moksha/client/anim/animation_system.h"
#include "moksha/client/audio/audio_system.h"
#include "moksha/client/debug/debug_system.h"
#include "moksha/client/game/game.h"
#include "moksha/client/game/game_factory.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/input/input_manager_win.h"
#include "moksha/client/misc/global_vars.h"
#include "moksha/client/net/content_client.h"
#include "moksha/client/net/game_client.h"
#include "moksha/client/rendering/directx/renderer_dx.h"
#include "moksha/client/ui/ui_system.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/ini/ini_parser.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/net/networking.h"
#include "moksha/shared/physics/physics_system.h"
#include "moksha/shared/time/time_manager.h"
#include "moksha/shared/time/timer_manager.h"

namespace ME {

class ClientApp {
   public:
    ClientApp();
    virtual ~ClientApp();

    ClientApp(const ClientApp&) = delete;
    ClientApp& operator=(const ClientApp&) = delete;
    ClientApp(ClientApp&&) = delete;
    ClientApp& operator=(ClientApp&&) = delete;

    void Init(HWND hwnd);
    void Update();
    void Exit();
    void ShutDownGameSystems();

    /** This feeds the windows OS input events to the game.
     * Game will dispatch it to Input System. */
    void HandleInput(UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    HWND hWnd;

    ME::Time::TimeManager timeManager;
    ME::Time::TimerManager timerManager;
    ME::Input::InputManager inputManager;
    ME::Input::InputManagerWin* winInputManager = nullptr;
    ME::GameClient gameClient;
    ME::ContentClient contentClient;
    // Created by the game's CreateGame() in the constructor, destroyed with DestroyGame() in the destructor.
    ME::Game* game = nullptr;
    ME::PhysicsSystem physicsSystem;
    ME::AnimationSystem animationSystem;
    ME::AudioSystem audioSystem;
    ME::UISystem uiSystem;
    ME::DebugSystem debugSystem;
    ME::RendererDX renderer;

    int fixedFrameRate = 0;
    bool vsync = true;
    // ShutDownGameSystems and the destructor both run; shut networking down only once.
    bool networkingStarted = false;
};

}  // namespace ME

#endif  // ME_WIN
