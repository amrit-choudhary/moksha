/**
 * Physics System.
 * This will handle the physics simulation, including collision detection and response.
 * It will also manage the physics scenes and their interactions.
 */

#pragma once

#include <cstdint>

#include "physics_scene.h"

namespace ME {

// Implemented by anything that wants to be notified when PhysicsSystem detects a collision.
// Keeps PhysicsSystem decoupled from any concrete game class.
class ICollisionListener {
   public:
    virtual ~ICollisionListener() = default;

    // By convention, the first collider is always the dynamic one (e.g., a ball), and the
    // second is static. Remember to delete the result after use.
    virtual void CollisionCallback(ColliderAABB* a, ColliderAABB* b, CollisionResultAABB* result) = 0;
};

class PhysicsSystem {
   public:
    // Initializes the physics system.
    void Init();

    // Updates the physics system with the given delta time.
    void Update(double deltaTime);

    // Ends the physics system, cleaning up resources if necessary.
    void End();

    // Set Scene for the physics system.
    void SetScene(PhysicsScene* physicsScene);

    // Set the listener to be notified when a collision is detected, if needed.
    void SetCollisionListener(ICollisionListener* listener);

    /**
     * Report collision between two colliders. Other systems can hook to this function to handle collisions.
     * By convention, the first collider is always the dynamic one (e.g., a ball), and the second is static.
     * result will store the collision result, such as penetration depth, normal vector, etc.
     * Remember to delete the result after use.
     */
    void ReportCollision(ColliderAABB* a, ColliderAABB* b, CollisionResultAABB* result);

   private:
    bool isInitialized = false;              // Flag to check if the system is initialized.
    PhysicsScene* scene = nullptr;           // Current physics scene being managed.
    ICollisionListener* listener = nullptr;  // Listener to notify on collision, if needed.

    /**
     * Setup collision categories for optimized collision checks.
     * This organizes colliders indices into categories based on their physics layers,
     */
    void SetupCollisionCategories();

    /**
     * Array of collider indices categorized by physics layer category.
     * This is used to optimize collision checks by only checking colliders
     * that belong to layers that can interact with each other.
     */
    uint32_t** categoryIndices = nullptr;
    /**
     * Count of colliders in each category index.
     */
    uint32_t* categoryCounts = nullptr;
};

}  // namespace ME
