/**
 * Collider class for physics interactions.
 */

#pragma once

#include <cstdint>

#include "physics_layer.h"

namespace ME {

/**
 * Collider class representing a physics collider.
 */
class Collider {
   public:
    Collider() : id(0), isEnabled(true), isStatic(true), category(PhysicsLayer::Default), mask(PhysicsLayer::All) {}
    Collider(uint32_t id, bool isEnabled, bool isStatic, PhysicsLayer category = PhysicsLayer::Default,
             PhysicsLayer mask = PhysicsLayer::All)
        : id(id), isEnabled(isEnabled), isStatic(isStatic), category(category), mask(mask) {}

    // Gets the ID of the collider.
    inline uint32_t GetID() const {
        return id;
    }

    bool isEnabled = false;
    bool isStatic = true;  // Indicates if the collider is static (not moving).

    // Physics layer category of the collider.
    PhysicsLayer category = PhysicsLayer::Default;

    // Physics layer mask for collision filtering.
    // Collisions will be checked only if (mask & other.category) != 0. for this and vice versa.
    PhysicsLayer mask = PhysicsLayer::All;

   private:
    uint32_t id = 0;  // Unique identifier for the collider.
};

}  // namespace ME
