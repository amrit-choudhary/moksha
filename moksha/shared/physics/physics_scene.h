/**
 * Physics scene, containing all the physics objects and their interactions.
 */

#pragma once

#include <cstdint>

#include "collider_aabb.h"
#include "moksha/shared/datastructure/span.h"
#include "moksha/shared/misc/engine_limits.h"

namespace ME {

/**
 * PhysicsScene class representing a physics scene.
 * It contains list of static and dynamic colliders.
 */
class PhysicsScene {
   public:
    PhysicsScene();
    virtual ~PhysicsScene();

    // Initializes the physics scene with collider objects.
    virtual void Init();

    // Initializes the physics scene with the given static/dynamic collider arrays.
    virtual void Init(ME::Span<ColliderAABB> inStaticColliders, ME::Span<ColliderAABB> inDynamicColliders);

    ME::Span<ColliderAABB> staticColliders;
    ME::Span<ColliderAABB> dynamicColliders;

    Collider* GetStaticColliderById(uint32_t id) const;
    Collider* GetDynamicColliderById(uint32_t id) const;

   private:
    uint32_t* mapStaticIdToIndex;
    uint32_t* mapDynamicIdToIndex;
};

}  // namespace ME
