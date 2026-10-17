#include "collider_aabb.h"

#include "moksha/shared/math/math.h"

ME::ColliderAABB::ColliderAABB(uint32_t id, bool isEnabled, bool isStatic, PhysicsLayer category, PhysicsLayer mask,
                               const ME::Transform& transform, float scaleMult)
    : Collider(id, isEnabled, isStatic, category, mask) {
    Vec3 pos = transform.GetPosition();
    Vec3 scale = transform.GetScale() * scaleMult;
    minX = pos.x - (scale.x / 2.0f);
    minY = pos.y - (scale.y / 2.0f);
    maxX = pos.x + (scale.x / 2.0f);
    maxY = pos.y + (scale.y / 2.0f);
}

void ME::ColliderAABB::UpdateTransform(const ME::Transform& transform, float scaleMult) {
    {
        Vec3 pos = transform.GetPosition();
        Vec3 scale = transform.GetScale() * scaleMult;
        minX = pos.x - (scale.x / 2.0f);
        minY = pos.y - (scale.y / 2.0f);
        maxX = pos.x + (scale.x / 2.0f);
        maxY = pos.y + (scale.y / 2.0f);
    }
}

ME::CollisionResultAABB* ME::ColliderAABB::GetCollisionResult(const ColliderAABB& other) const {
    if (!CheckCollision(other)) return nullptr;

    CollisionResultAABB* result = new CollisionResultAABB();

    // Calculate the penetration depth in both X and Y directions.
    // Penetration in x direction is the distance between the rightmost left edges and the leftmost right edges.
    // Penetration is always a positive value.
    result->penetration.x = Math::Min(maxX, other.maxX) - Math::Max(minX, other.minX);
    result->penetration.y = Math::Min(maxY, other.maxY) - Math::Max(minY, other.minY);

    // Take the minimum penetration depth to determine the separation distance.
    if (result->penetration.x < result->penetration.y) {
        // Should separate in the X direction.
        result->seperation.y = 0.0f;

        if (Math::Max(minX, other.minX) == minX) {
            // first collider is on the right side of the second collider.
            result->seperation.x = result->penetration.x;
            result->normal = Vec2::Right;
        } else {
            // first collider is on the left side of the second collider.
            result->seperation.x = -result->penetration.x;
            result->normal = Vec2::Left;
        }
    } else {
        // Should separate in the Y direction.
        result->seperation.x = 0.0f;

        if (Math::Max(minY, other.minY) == minY) {
            // first collider is above the second collider.
            result->seperation.y = result->penetration.y;
            result->normal = Vec2::Up;
        } else {
            // first collider is below the second collider.
            result->seperation.y = -result->penetration.y;
            result->normal = Vec2::Down;
        }
    }

    return result;
}
