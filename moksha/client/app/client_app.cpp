#if defined(ME_WIN) || defined(ME_MAC) || defined(ME_IOS) || defined(ME_HEADLESS)

#include "client_app.h"

#include <cstdio>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/ini/ini_parser.h"
#include "moksha/shared/net/networking.h"

ME::ClientApp::ClientApp() {}

ME::ClientApp::~ClientApp() {
    Shutdown();
    if (game != nullptr) {
        ME::DestroyGame(game);
        game = nullptr;
    }
}

const ME::AppConfig& ME::ClientApp::Configure(const AppArgs& args) {
    config = AppConfig{};
    game = ME::CreateGame(args, config);
    config.ApplySettings(ME::INIParser::Load());
    config.ApplyArgs(args);
    return config;
}

#if defined(ME_DX12)
bool ME::ClientApp::Init(HWND hWnd) {
#elif defined(ME_METAL)
bool ME::ClientApp::Init(const MetalView* view) {
#else
bool ME::ClientApp::Init() {
#endif
    if (game == nullptr) {
        ME::LogError("ClientApp: CreateGame returned no game");
        return false;
    }

    window.width = config.windowWidth;
    window.height = config.windowHeight;

    engine.time = &timeManager;
    engine.timers = &timerManager;
    engine.input = &inputManager;
    engine.physics = &physicsSystem;
    engine.animation = &animationSystem;
    engine.audio = &audioSystem;
    engine.ui = &uiSystem;
    engine.gameClient = &gameClient;
    engine.contentClient = &contentClient;
    engine.config = &config;
    engine.window = &window;

    debugSystem.Init(&window);
    ME::DebugSystem::SetInstance(&debugSystem);

    inputManager.Init(&window);
#if defined(ME_WIN) && !defined(ME_HEADLESS)
    winInputManager = static_cast<ME::Input::InputManagerWin*>(inputManager.GetPlatformInputManager());
#elif defined(ME_MAC) && !defined(ME_HEADLESS)
    macInputManager = static_cast<ME::Input::InputManagerMac*>(inputManager.GetPlatformInputManager());
#endif
    // Once per process, before any socket (game connection, content sync) is created.
    networkingStarted = ME::Net::InitNetworking();
    gameClient.Init(config.serverIP.c_str(), config.serverPort);
    physicsSystem.Init();
    animationSystem.Init();
    audioSystem.Init();
    // Before game->Init, so the game can schedule timers from Init/Start.
    timerManager.Init(config.fixedStepFPS);
    uiSystem.Init(&window);

    initialized = true;
    game->Init(engine);

    uiSystem.SetUIScene(game->GetUIScene());

#if defined(ME_DX12)
    renderer.InitDX(hWnd, &window);
#elif defined(ME_METAL)
    renderer.InitMetal(view, &window);
#else
    renderer.InitNull(&window);
#endif
    renderer.SetVsyncEnabled(config.vsync);
    renderer.SetScenes(game->GetScene(), game->GetUIScene());
    renderer.SetDebugUIScene(debugSystem.GetUIScene());

    audioSystem.SetScene(game->GetScene());

    // Clock init after all systems are initialized.
    ME::Time::TimeConfig timeConfig;
    timeConfig.fixedStepFPS = config.fixedStepFPS;
    timeManager.Init(timeConfig);

    // Content sync runs in the background from here on; the game never waits for it.
    contentClient.Init(config.contentServerIP.c_str(), config.contentServerPort, timeManager.GetTimeSinceStartup());

    // Start the game.
    game->Start();
    return true;
}

bool ME::ClientApp::Tick() {
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

    if (!paused) {
        renderer.Update();
        renderer.Draw();
    }

    gameClient.Update(deltaTime);
    contentClient.Update(timeManager.GetTimeSinceStartup());
    audioSystem.Update(deltaTime);

    return !engine.IsQuitRequested();
}

void ME::ClientApp::Shutdown() {
    if (!initialized) {
        return;
    }
    initialized = false;

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

void ME::ClientApp::Pause() {
    if (!initialized || paused) {
        return;
    }
    paused = true;
    game->OnPause();
}

void ME::ClientApp::Resume() {
    if (!initialized || !paused) {
        return;
    }
    paused = false;
    game->OnResume();
}

const ME::Time::TimeManager& ME::ClientApp::GetTime() const {
    return timeManager;
}

#if defined(ME_WIN) && !defined(ME_HEADLESS)
void ME::ClientApp::HandleInput(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (winInputManager != nullptr) {
        winInputManager->HandleInput(msg, wParam, lParam);
    }
}
#elif defined(ME_MAC) && !defined(ME_HEADLESS)
void ME::ClientApp::HandleKeyEvent(uint16_t keyCode, bool isDown) {
    if (macInputManager != nullptr) {
        macInputManager->HandleKeyEvent(keyCode, isDown);
    }
}

void ME::ClientApp::HandleMouseMove(float x, float y) {
    if (macInputManager != nullptr) {
        macInputManager->HandleMouseMove(x, y);
    }
}

void ME::ClientApp::HandleMouseButton(int button, bool isDown) {
    if (macInputManager != nullptr) {
        macInputManager->HandleMouseButton(button, isDown);
    }
}
#endif

#endif  // ME_WIN || ME_MAC || ME_IOS || ME_HEADLESS
