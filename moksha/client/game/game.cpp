#include "game.h"

#include <cmath>
#include <iostream>

#include "moksha/client/debug/debug_system.h"
#include "moksha/client/input/input_manager.h"

using namespace ME;
using namespace ME::Input;

Game::Game() {}

Game::~Game() {}

void Game::Init(ME::Time::TimeManager* currentTimeManager) {
    ME::Log("Game Start!");

    timeManager = currentTimeManager;

    ME::DebugSystem::ScreenPrintSlot(1, GetDisplayName());
}

void ME::Game::Start() {}

void Game::Update(double deltaTime) {}

void Game::FixedUpdate(double fixedDeltaTime) {}

void Game::End() {
    std::cout << "Game Run Time: " << timeManager->GetTimeSinceStartup() << '\n';
    std::cout << "Average Frame Rate: " << timeManager->GetFrameCount() / timeManager->GetTimeSinceStartup() << '\n';
    std::cout << "Average Sim Tick Rate: " << timeManager->GetFixedStepCount() / timeManager->GetTimeSinceStartup()
              << '\n';
    std::cout << "Total Frames: " << timeManager->GetFrameCount() << '\n';

    ME::Log("Game Over!");
}

void ME::Game::SetInputManagerRef(ME::Input::InputManager* ptrInput) {
    inputManager = ptrInput;
}

void ME::Game::SetGameClientRef(ME::GameClient* ptrGameClient) {
    gameClient = ptrGameClient;
}

void ME::Game::SetPhysicsSystemRef(ME::PhysicsSystem* ptrPhysicsSystem) {
    physicsSystem = ptrPhysicsSystem;
}

void ME::Game::SetAnimationSystemRef(ME::AnimationSystem* ptrAnimationSystem) {
    animationSystem = ptrAnimationSystem;
}

void ME::Game::SetAudioSystemRef(ME::AudioSystem* ptrAudioSystem) {
    audioSystem = ptrAudioSystem;
}

void ME::Game::SetUISystemRef(ME::UISystem* ptrUISystem) {
    uiSystem = ptrUISystem;
}

void ME::Game::SetTimerManagerRef(ME::Time::TimerManager* ptrTimerManager) {
    timerManager = ptrTimerManager;
}

void ME::Game::CollisionCallback(ColliderAABB* a, ColliderAABB* b, CollisionResultAABB* result) {
    delete result;
}

ME::Scene* ME::Game::GetScene() {
    return scene;
}

ME::SceneUI* ME::Game::GetUIScene() {
    return uiScene;
}

const char* ME::Game::GetDisplayName() const {
    return "Game";
}
