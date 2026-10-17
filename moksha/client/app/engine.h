/**
 * What a game gets from the engine: every client system plus the app's configuration and window. ClientApp owns
 * the systems and hands this to Game::Init; it is passed in, never reached through a global.
 */
#pragma once

namespace ME {

class AnimationSystem;
class AppConfig;
class AudioSystem;
class ContentClient;
class GameClient;
class PhysicsSystem;
class UISystem;
class WindowState;

namespace Input {
class InputManager;
}

namespace Time {
class TimeManager;
class TimerManager;
}  // namespace Time

class Engine {
   public:
    Time::TimeManager* time = nullptr;
    Time::TimerManager* timers = nullptr;
    Input::InputManager* input = nullptr;
    PhysicsSystem* physics = nullptr;
    AnimationSystem* animation = nullptr;
    AudioSystem* audio = nullptr;
    UISystem* ui = nullptr;
    GameClient* gameClient = nullptr;
    ContentClient* contentClient = nullptr;

    // The final startup configuration, after settings.ini and the command line.
    const AppConfig* config = nullptr;

    // The window the app actually got.
    const WindowState* window = nullptr;

    // Asks the app to stop after the current frame. On platforms that never exit (iOS, web) it is only a request.
    void Quit();
    bool IsQuitRequested() const;

    // True in the windowless client build (no renderer, no input, no audio).
    bool IsHeadless() const;

   private:
    bool quitRequested = false;
};

}  // namespace ME
