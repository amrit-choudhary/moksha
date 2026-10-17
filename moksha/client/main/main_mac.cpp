#ifdef ME_MAC

#include "main_mac.h"

#include <iostream>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

ME::ClientApp::ClientApp() {
    game = ME::CreateGame();
}

ME::ClientApp::~ClientApp() {
    game->End();
    gameClient.End();
    if (networkingStarted) {
        ME::Net::ShutdownNetworking();
        networkingStarted = false;
    }
    inputManager.End();
    timeManager.End();
    renderer.End();
    physicsSystem.End();
    uiSystem.End();

    ME::DestroyGame(game);
    game = nullptr;
}

void ME::ClientApp::Init(MTL::Device* device, MTK::View* view) {
    // Read game params from file.
    ME::INIMap iniMap = ME::INIParser::Load();
    fps = std::atoi(iniMap["settings"]["fps"].c_str());

    std::string serverIP = iniMap["settings"]["serverIP"];
    if (serverIP.empty()) serverIP = "127.0.0.1";
    std::string serverPortStr = iniMap["settings"]["serverPort"];
    uint16_t serverPort = serverPortStr.empty() ? 9310 : static_cast<uint16_t>(std::atoi(serverPortStr.c_str()));

    inputManager.Init();
    macInputManager = static_cast<ME::Input::InputManagerMac*>(inputManager.GetPlatformInputManager());
    // Once per process, before any socket is created.
    networkingStarted = ME::Net::InitNetworking();
    gameClient.Init(serverIP.c_str(), serverPort);
    physicsSystem.Init();

    game->SetInputManagerRef(&inputManager);
    game->SetGameClientRef(&gameClient);
    game->SetPhysicsSystemRef(&physicsSystem);
    game->Init(&timeManager);

    uiSystem.Init();
    uiSystem.SetUIScene(game->GetUIScene());
    game->SetUISystemRef(&uiSystem);

    renderer.InitMTL(device, view);
    renderer.SetScene(game->GetScene());

    // Clock init after all systems are initialized.
    timeManager.Init(fps, false);
    bool shouldTick = false;
    double deltaTime = 0.0f;

    game->Start();
}

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

void ME::ClientApp::Update() {
    bool shouldTick = timeManager.Update();
    double deltaTime = 0.0f;

    if (shouldTick) {
        deltaTime = timeManager.GetDeltaTime();

        inputManager.PreUpdate();
        inputManager.Update(deltaTime);

        game->Update(deltaTime);
        uiSystem.Update(deltaTime);

        inputManager.PostUpdate();

        renderer.Update();
        renderer.Draw();

        gameClient.Update(deltaTime);

        physicsSystem.Update(deltaTime);
    }
}

void ME::ClientApp::Exit() {
    // Clean up game systems here
    std::cout << "Game exited." << std::endl;
}

void ME::ClientApp::ShutDownGameSystems() {
    game->End();
    gameClient.End();
    if (networkingStarted) {
        ME::Net::ShutdownNetworking();
        networkingStarted = false;
    }
    inputManager.End();
    timeManager.End();
    renderer.End();
    physicsSystem.End();
    uiSystem.End();
}

#endif  // ME_MAC