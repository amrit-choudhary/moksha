/**
 * Runs a game server process: exe paths, settings, networking, the fixed-step loop and the GameServer.
 * A game supplies an IGameServerListener; its main() is usually just `return app.Run(argc, argv, listener);`.
 */
#pragma once

#include "moksha/server/game_server.h"
#include "moksha/shared/time/time_manager.h"
#include "moksha/shared/time/timer_manager.h"

namespace ME {

class ServerApp {
   public:
    ServerApp() = default;
    ~ServerApp() = default;

    ServerApp(const ServerApp&) = delete;
    ServerApp& operator=(const ServerApp&) = delete;
    ServerApp(ServerApp&&) = delete;
    ServerApp& operator=(ServerApp&&) = delete;

    // Init, then Tick until a quit is requested, sleeping between fixed steps, then Shutdown. Returns the exit code.
    int Run(int argc, char** argv, IGameServerListener& listener);

    // Sets up paths, time, timers, networking and the GameServer, then calls listener.OnStart.
    // Call Shutdown afterwards whether or not it succeeded.
    bool Init(int argc, char** argv, IGameServerListener& listener);

    // Runs every due fixed step: timers first, then the GameServer. False once a quit has been requested.
    bool Tick();

    // Calls listener.OnEnd and releases everything Init set up. Safe after a failed or partial Init.
    void Shutdown();

    // Ends the loop after the current Tick.
    void RequestQuit();

    GameServer& GetGameServer();
    Time::TimerManager& GetTimers();
    const Time::TimeManager& GetTime() const;

   private:
    Time::TimeManager time;
    Time::TimerManager timers;
    GameServer gameServer;
    IGameServerListener* listener = nullptr;

    bool timersStarted = false;
    bool networkingStarted = false;
    bool serverStarted = false;
    bool quitRequested = false;
};

}  // namespace ME
