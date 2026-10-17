#include "physics_system.h"

void ME::PhysicsSystem::Init() {
    // Init the category indices and count for collision optimization.
    categoryIndices = new uint32_t*[ME::Constants::MaxPhysicsLayerCount];
    for (size_t i = 0; i < ME::Constants::MaxPhysicsLayerCount; ++i) {
        categoryIndices[i] = new uint32_t[ME::Constants::MaxDynamicColliderCount];
    }

    categoryCounts = new uint32_t[ME::Constants::MaxPhysicsLayerCount];
    for (size_t i = 0; i < ME::Constants::MaxPhysicsLayerCount; ++i) {
        categoryCounts[i] = 0;
    }
}

void ME::PhysicsSystem::Update(double deltaTime) {
    if (scene == nullptr || !isInitialized) {
        return;
    }

    /**
     * Basic NxN collision detection between dynamic colliders only.
     * This is optimized by categorizing colliders based on their physics layers at startup.
     * Only colliders that belong to layers that can interact with each other are checked for collisions.
     * Steps:
     * 1. For each category, get the list of dynamic colliders in that category.
     * 2. For each collider in that category, check against colliders in other categories
     *   that are allowed to collide with it based on their masks.
     */
    for (uint8_t categoryIndex = 0; categoryIndex < ME::Constants::MaxPhysicsLayerCount; ++categoryIndex) {
        // Colliders in this category.
        uint32_t count = categoryCounts[categoryIndex];

        for (uint32_t i = 0; i < count; ++i) {
            uint32_t dynamicColliderIndexA = categoryIndices[categoryIndex][i];
            ColliderAABB* dynamicColliderA = &scene->dynamicColliders[dynamicColliderIndexA];

            if (!dynamicColliderA->isEnabled) {
                continue;
            }

            // Start checking from next category to avoid double checking.
            for (uint8_t otherCategoryIndex = (categoryIndex + 1);
                 otherCategoryIndex < ME::Constants::MaxPhysicsLayerCount; ++otherCategoryIndex) {
                if (categoryIndex == otherCategoryIndex) {
                    continue;  // Skip same category to avoid double checking.
                }

                PhysicsLayer otherCategoryLayer = ME::Physics::GetPhysicsLayerFromIndex(otherCategoryIndex);
                if (ME::Physics::HasLayer(dynamicColliderA->mask, otherCategoryLayer) == false) {
                    continue;  // No collision allowed between these layers.
                }

                uint32_t otherCount = categoryCounts[otherCategoryIndex];
                for (uint32_t j = 0; j < otherCount; ++j) {
                    uint32_t dynamicColliderIndexB = categoryIndices[otherCategoryIndex][j];
                    ColliderAABB* dynamicColliderB = &scene->dynamicColliders[dynamicColliderIndexB];

                    if (!dynamicColliderB->isEnabled) {
                        continue;
                    }

                    if (!ME::Physics::IsCollisionAllowed(dynamicColliderA->category, dynamicColliderA->mask,
                                                         dynamicColliderB->category, dynamicColliderB->mask)) {
                        continue;
                    }

                    CollisionResultAABB* result = dynamicColliderA->GetCollisionResult(*dynamicColliderB);
                    if (result == nullptr) {
                        continue;  // No collision detected.
                    } else {
                        ReportCollision(dynamicColliderA, dynamicColliderB, result);
                    }
                }
            }
        }
    }
}

void ME::PhysicsSystem::End() {
    for (size_t i = 0; i < ME::Constants::MaxPhysicsLayerCount; ++i) {
        delete[] categoryIndices[i];
        categoryIndices[i] = nullptr;
    }
    delete[] categoryIndices;
    categoryIndices = nullptr;
    delete[] categoryCounts;
    categoryCounts = nullptr;

    isInitialized = false;
}

void ME::PhysicsSystem::SetScene(ME::PhysicsScene* physicsScene) {
    scene = physicsScene;
    SetupCollisionCategories();
    isInitialized = true;
}

void ME::PhysicsSystem::SetCollisionListener(ME::ICollisionListener* listener) {
    this->listener = listener;
}

void ME::PhysicsSystem::ReportCollision(ColliderAABB* a, ColliderAABB* b, CollisionResultAABB* result) {
    if (listener != nullptr) {
        listener->CollisionCallback(a, b, result);
    } else {
        delete result;  // No listener to report to, just delete the result.
    }
}

void ME::PhysicsSystem::SetupCollisionCategories() {
    for (size_t i = 0; i < ME::Constants::MaxPhysicsLayerCount; ++i) {
        categoryCounts[i] = 0;
    }

    for (size_t i = 0; i < scene->dynamicColliders.count; ++i) {
        ColliderAABB* dynamicCollider = &scene->dynamicColliders[i];
        if (dynamicCollider->isStatic) {
            continue;
        }
        uint8_t layerIndex = ME::Physics::GetIndexFromPhysicsLayer(dynamicCollider->category);
        uint32_t count = categoryCounts[layerIndex];
        categoryIndices[layerIndex][count] = i;
        categoryCounts[layerIndex] = count + 1;
    }
}
