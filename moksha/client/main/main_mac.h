#ifdef ME_MAC

/**
 * Game Engine side Main Entry Point for macOS.
 * Since mac entry is a bit different, we have a simplified main entry point.
 */

#pragma once

#include <cassert>

#include "moksha/client/game/game.h"
#include "moksha/client/game/game_factory.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/input/input_manager_mac.h"
#include "moksha/client/misc/global_vars.h"
#include "moksha/client/net/game_client.h"
#include "moksha/client/rendering/metal/renderer_metal.h"
#include "moksha/client/ui/ui_system.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/ini/ini_parser.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/net/networking.h"
#include "moksha/shared/physics/physics_system.h"
#include "moksha/shared/time/time_manager.h"

namespace ME {

class ClientApp {
   public:
    ClientApp();
    virtual ~ClientApp();

    // Receives the Metal device/view once at boot, mirroring ClientApp::Init(HWND) on Windows.
    void Init(MTL::Device* device, MTK::View* view);
    void Update();
    void Exit();
    void ShutDownGameSystems();

    /** Feeds macOS input events to the game. Game dispatches it to the Input System. */
    void HandleKeyEvent(uint16_t keyCode, bool isDown);
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(int button, bool isDown);

   private:
    // Game Systems
    ME::Time::TimeManager timeManager;
    ME::Input::InputManager inputManager;
    ME::Input::InputManagerMac* macInputManager = nullptr;
    ME::GameClient gameClient;
    // Created by the game's CreateGame() in the constructor, destroyed with DestroyGame() in the destructor.
    ME::Game* game = nullptr;
    ME::RendererMetal renderer;
    ME::PhysicsSystem physicsSystem;
    ME::UISystem uiSystem;

    int fps = 0;
    // ShutDownGameSystems and the destructor both run; shut networking down only once.
    bool networkingStarted = false;
};

}  // namespace ME

#endif  // ME_MAC
