#pragma once

/**
 * Animation System.
 * This system manages all animations within the game.
 * This will call update on all sprite animators in the current scene.
 */

#include <cstdint>

namespace ME {

// Forward declarations.
class Scene;

class AnimationSystem {
   public:
    // Initializes the animation system.
    void Init();

    // Updates the animation system with the given delta time.
    void Update(double deltaTime);

    // Ends the animation system, cleaning up resources if necessary.
    void End();

    // Set Scene for the animation system.
    void SetScene(Scene* scene);

   private:
    bool isInitialized = false;  // Flag to check if the system is initialized.
    ME::Scene* scene = nullptr;  // Current scene being managed.
};

}  // namespace ME
