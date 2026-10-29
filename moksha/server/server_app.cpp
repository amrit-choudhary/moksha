#include "server_app.h"

#include <cstdlib>
#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/ini/ini_parser.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/net/networking.h"
#include "moksha/shared/platform/exe_path.h"
#include "moksha/shared/time/frame_pacer.h"

int ME::ServerApp::Run(int argc, char** argv, IGameServerListener& inListener) {
    if (!Init(argc, argv, inListener)) {
        Shutdown();
        return 1;
    }
    while (Tick()) {
        Time::WaitForNextFixedStep(time);
    }
    Shutdown();
    return 0;
}

bool ME::ServerApp::Init(int argc, char** argv, IGameServerListener& inListener) {
    listener = &inListener;
    quitRequested = false;

    std::string exePath = Platform::GetExecutablePath(argc > 0 ? argv[0] : nullptr);
    Utils::SetPaths(Utils::MakeExecutableRelativePaths(exePath));

    Time::TimeConfig timeConfig;
    timeConfig.fixedStepFPS = Time::FPS_60;
    time.Init(timeConfig);
    timers.Init(timeConfig.fixedStepFPS);
    timersStarted = true;

    INIMap iniMap = INIParser::Load();
    const std::string portStr = iniMap["settings"]["port"];
    const uint16_t port =
        portStr.empty() ? DEFAULT_GAME_SERVER_PORT : static_cast<uint16_t>(std::atoi(portStr.c_str()));

    // Once per process, before any socket is created.
    if (!Net::InitNetworking()) {
        return false;
    }
    networkingStarted = true;

    if (!gameServer.Init(port, listener)) {
        return false;
    }
    serverStarted = true;

    listener->OnStart(*this);
    return true;
}

bool ME::ServerApp::Tick() {
    time.BeginFrame();

    const int steps = time.GetPendingFixedSteps();
    for (int i = 0; i < steps; ++i) {
        const double fixedDeltaTime = time.GetFixedDeltaTime();

        // First in the step, so timers scheduled during step N fire at the start of step N+1.
        timers.Tick(fixedDeltaTime);
        gameServer.Update(fixedDeltaTime);
    }
    return !quitRequested;
}

void ME::ServerApp::Shutdown() {
    if (serverStarted) {
        listener->OnEnd();
        gameServer.End();
        serverStarted = false;
    }
    // After the game's own cleanup, so it can still clear its handles; drops any it left behind.
    if (timersStarted) {
        timers.End();
        timersStarted = false;
    }
    if (networkingStarted) {
        Net::ShutdownNetworking();
        networkingStarted = false;
    }
    time.End();
    listener = nullptr;
}

void ME::ServerApp::RequestQuit() {
    quitRequested = true;
}

ME::GameServer& ME::ServerApp::GetGameServer() {
    return gameServer;
}

ME::Time::TimerManager& ME::ServerApp::GetTimers() {
    return timers;
}

const ME::Time::TimeManager& ME::ServerApp::GetTime() const {
    return time;
}
