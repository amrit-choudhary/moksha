#ifdef ME_WIN

#include "main_win.h"

#include <cstdio>
#include <iostream>

ME::ClientApp::ClientApp() {
    game = ME::CreateGame();
}

ME::ClientApp::~ClientApp() {
    game->End();
    // After the game ends, so its own cleanup can still clear handles; drops any it left behind.
    timerManager.End();
    gameClient.End();
    contentClient.End();
    if (networkingStarted) {
        ME::Net::ShutdownNetworking();
        networkingStarted = false;
    }
    inputManager.End();
    timeManager.End();
    physicsSystem.End();
    animationSystem.End();
    audioSystem.End();
    uiSystem.End();
    renderer.End();

    ME::DebugSystem::SetInstance(nullptr);
    debugSystem.End();

    ME::DestroyGame(game);
    game = nullptr;
}

void ME::ClientApp::Init(HWND hWnd) {
    this->hWnd = hWnd;

    // Read game params from file.
    ME::INIMap iniMap = ME::INIParser::Load();
    fixedFrameRate = std::atoi(iniMap["settings"]["fixedFrameRate"].c_str());
    vsync = std::atoi(iniMap["settings"]["vsync"].c_str()) != 0;

    std::string serverIP = iniMap["settings"]["serverIP"];
    if (serverIP.empty()) serverIP = "127.0.0.1";
    std::string serverPortStr = iniMap["settings"]["serverPort"];
    uint16_t serverPort = serverPortStr.empty() ? 9310 : static_cast<uint16_t>(std::atoi(serverPortStr.c_str()));

    std::string contentServerIP = iniMap["settings"]["contentServerIP"];
    if (contentServerIP.empty()) contentServerIP = "127.0.0.1";
    std::string contentServerPortStr = iniMap["settings"]["contentServerPort"];
    uint16_t contentServerPort =
        contentServerPortStr.empty() ? 9311 : static_cast<uint16_t>(std::atoi(contentServerPortStr.c_str()));

    debugSystem.Init();
    ME::DebugSystem::SetInstance(&debugSystem);

    inputManager.Init();
    winInputManager = static_cast<ME::Input::InputManagerWin*>(inputManager.GetPlatformInputManager());
    // Once per process, before any socket (game connection, content sync) is created.
    networkingStarted = ME::Net::InitNetworking();
    gameClient.Init(serverIP.c_str(), serverPort);
    physicsSystem.Init();
    animationSystem.Init();
    audioSystem.Init();
    // Before game->Init, so the game can schedule timers from Init/Start.
    timerManager.Init(static_cast<double>(fixedFrameRate));

    game->SetInputManagerRef(&inputManager);
    game->SetGameClientRef(&gameClient);
    game->SetPhysicsSystemRef(&physicsSystem);
    game->SetAnimationSystemRef(&animationSystem);
    game->SetAudioSystemRef(&audioSystem);
    game->SetTimerManagerRef(&timerManager);
    game->Init(&timeManager);

    uiSystem.Init();
    uiSystem.SetUIScene(game->GetUIScene());
    game->SetUISystemRef(&uiSystem);

    renderer.InitDX(hWnd);
    renderer.SetVsyncEnabled(vsync);
    renderer.SetScenes(game->GetScene(), game->GetUIScene());
    renderer.SetDebugUIScene(debugSystem.GetUIScene());

    audioSystem.SetScene(game->GetScene());

    // Clock init after all systems are initialized.
    ME::Time::TimeConfig timeConfig;
    timeConfig.fixedStepFPS = static_cast<double>(fixedFrameRate);
    timeManager.Init(timeConfig);

    // Content sync runs in the background from here on; the game never waits for it.
    contentClient.Init(contentServerIP.c_str(), contentServerPort, timeManager.GetTimeSinceStartup());

    // Start the game.
    game->Start();
}

void ME::ClientApp::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (winInputManager != nullptr) {
        winInputManager->HandleInput(msg, wParam, lParam);
    }
}

void ME::ClientApp::Update() {
    timeManager.BeginFrame();
    double deltaTime = timeManager.GetFrameDeltaTime();

    char fpsBuf[64];
    snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.0f", timeManager.GetCurrentFPS());
    ME::DebugSystem::ScreenPrintSlot(0, fpsBuf);

    inputManager.PreUpdate();
    inputManager.Update(deltaTime);

    for (int i = 0; i < timeManager.GetPendingFixedSteps(); ++i) {
        double fixedDeltaTime = timeManager.GetFixedDeltaTime();

        // First in the step, so timers scheduled during step N fire at the start of step N+1.
        timerManager.Tick(fixedDeltaTime);
        game->FixedUpdate(fixedDeltaTime);
        physicsSystem.Update(fixedDeltaTime);
        animationSystem.Update(fixedDeltaTime);
    }

    game->Update(deltaTime);
    uiSystem.Update(deltaTime);
    debugSystem.Update(deltaTime);

    inputManager.PostUpdate();

    renderer.Update();
    renderer.Draw();

    gameClient.Update(deltaTime);
    contentClient.Update(timeManager.GetTimeSinceStartup());
    audioSystem.Update(deltaTime);
}

void ME::ClientApp::Exit() {
    // Clean up game systems here
    std::cout << "Game exited." << std::endl;
}

void ME::ClientApp::ShutDownGameSystems() {
    game->End();
    // After the game ends, so its own cleanup can still clear handles; drops any it left behind.
    timerManager.End();
    gameClient.End();
    contentClient.End();
    if (networkingStarted) {
        ME::Net::ShutdownNetworking();
        networkingStarted = false;
    }
    inputManager.End();
    timeManager.End();
    physicsSystem.End();
    animationSystem.End();
    audioSystem.End();
    uiSystem.End();
    renderer.End();

    ME::DebugSystem::SetInstance(nullptr);
    debugSystem.End();
}

#endif  // ME_WIN
