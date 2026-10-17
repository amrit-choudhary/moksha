/**
 * Main Game class.
 * Derive this class for your own game.
 */

#pragma once

#include <cstdint>

#include "moksha/client/anim/animation_system.h"
#include "moksha/client/audio/audio_system.h"
#include "moksha/client/input/input_manager.h"
#include "moksha/client/net/game_client.h"
#include "moksha/client/scene/scene.h"
#include "moksha/client/scene/scene_ui.h"
#include "moksha/client/ui/ui_system.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/datastructure/ring_buffer.h"
#include "moksha/shared/math/math.h"
#include "moksha/shared/physics/physics_system.h"
#include "moksha/shared/random/random_engine.h"
#include "moksha/shared/time/time_manager.h"
#include "moksha/shared/time/timer_manager.h"

namespace ME {

class Game : public ICollisionListener {
   public:
    Game();
    virtual ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    // Game control functions.

    // Init game
    virtual void Init(ME::Time::TimeManager* currentTimeManager);

    // Start game. This is called after all systems are initialized and before Update.
    virtual void Start();

    // Update game. Called once per real frame with a variable delta time -
    // input/UI/render-facing logic that doesn't need a constant dt.
    virtual void Update(double deltaTime);

    // Fixed-timestep update. Called 0, 1, or several times per real frame with a constant
    // delta time (ME::Time::TimeManager::GetFixedDeltaTime()) - gameplay/physics logic that
    // needs to behave the same regardless of render frame rate.
    virtual void FixedUpdate(double fixedDeltaTime);

    // End game.
    virtual void End();

    void SetInputManagerRef(ME::Input::InputManager* ptrInput);
    void SetGameClientRef(ME::GameClient* ptrGameClient);
    void SetPhysicsSystemRef(ME::PhysicsSystem* ptrPhysicsSystem);
    void SetAnimationSystemRef(ME::AnimationSystem* ptrAnimationSystem);
    void SetAudioSystemRef(ME::AudioSystem* ptrAudioSystem);
    void SetUISystemRef(ME::UISystem* ptrUISystem);
    void SetTimerManagerRef(ME::Time::TimerManager* ptrTimerManager);

    // This will be called from the PhysicsSystem when a collision is detected.
    // Remember to delete the result after use.
    void CollisionCallback(ColliderAABB* a, ColliderAABB* b, CollisionResultAABB* result) override;

    ME::Scene* GetScene();
    ME::SceneUI* GetUIScene();

    // Human-readable label for this game, shown on the debug overlay. Override to customize.
    virtual const char* GetDisplayName() const;

   protected:
    ME::Time::TimeManager* timeManager = nullptr;
    ME::Time::TimerManager* timerManager = nullptr;
    ME::GameClient* gameClient = nullptr;
    ME::Input::InputManager* inputManager = nullptr;
    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::PhysicsSystem* physicsSystem = nullptr;
    ME::AnimationSystem* animationSystem = nullptr;
    ME::AudioSystem* audioSystem = nullptr;
    ME::UISystem* uiSystem = nullptr;
};
}  // namespace ME
