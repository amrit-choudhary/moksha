#include "physics_scene.h"

ME::PhysicsScene::PhysicsScene() {}

ME::PhysicsScene::~PhysicsScene() {
    delete[] mapStaticIdToIndex;
    delete[] mapDynamicIdToIndex;
}

void ME::PhysicsScene::Init() {
    mapStaticIdToIndex = new uint32_t[Constants::MaxStaticColliderCount];
    mapDynamicIdToIndex = new uint32_t[Constants::MaxDynamicColliderCount];
}

void ME::PhysicsScene::Init(ME::Span<ColliderAABB> inStaticColliders, ME::Span<ColliderAABB> inDynamicColliders) {
    Init();

    staticColliders = inStaticColliders;
    dynamicColliders = inDynamicColliders;

    // TODO FIX sparse array.

    // for (size_t i = 0; i < staticColliders.count; ++i) {
    //     mapStaticIdToIndex[staticColliders[i].GetID()] = i;
    // }
    // for (size_t i = 0; i < dynamicColliders.count; ++i) {
    //     mapDynamicIdToIndex[dynamicColliders[i].GetID()] = i;
    // }
}

ME::Collider* ME::PhysicsScene::GetStaticColliderById(uint32_t id) const {
    if (id < staticColliders.count) {
        return &staticColliders[mapStaticIdToIndex[id]];
    } else {
        return nullptr;
    }
}

ME::Collider* ME::PhysicsScene::GetDynamicColliderById(uint32_t id) const {
    if (id < dynamicColliders.count) {
        return &dynamicColliders[mapDynamicIdToIndex[id]];
    } else {
        return nullptr;
    }
}
