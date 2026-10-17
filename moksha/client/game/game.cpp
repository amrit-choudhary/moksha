#include "game.h"

#include <cmath>
#include <iostream>

#include "moksha/client/debug/debug_system.h"
#include "moksha/client/input/input_manager.h"

using namespace ME;
using namespace ME::Input;

Game::Game() {}

Game::~Game() {}

void Game::Init(ME::Engine& inEngine) {
    ME::Log("Game Start!");

    engine = &inEngine;

    ME::DebugSystem::ScreenPrintSlot(1, GetDisplayName());
}

void ME::Game::Start() {}

void Game::Update(double deltaTime) {}

void Game::FixedUpdate(double fixedDeltaTime) {}

void Game::End() {
    const ME::Time::TimeManager* timeManager = engine != nullptr ? engine->time : nullptr;
    if (timeManager != nullptr && timeManager->GetTimeSinceStartup() > 0.0) {
        std::cout << "Game Run Time: " << timeManager->GetTimeSinceStartup() << '\n';
        std::cout << "Average Frame Rate: " << timeManager->GetFrameCount() / timeManager->GetTimeSinceStartup()
                  << '\n';
        std::cout << "Average Sim Tick Rate: " << timeManager->GetFixedStepCount() / timeManager->GetTimeSinceStartup()
                  << '\n';
        std::cout << "Total Frames: " << timeManager->GetFrameCount() << '\n';
    }

    ME::Log("Game Over!");
}

void ME::Game::OnPause() {}

void ME::Game::OnResume() {}

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
