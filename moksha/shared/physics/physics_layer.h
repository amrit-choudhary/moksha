#pragma once

/**
 * This defines the different physics layers used in the physics simulation.
 * Each layer can be used to categorize objects for collision detection and response.
 * Layers can be combined using bitwise operations to create complex collision rules.
 */

#include <cstdint>

namespace ME {

/**
 * Define the physics layers using bit flags.
 * Use this to define category and mask.
 * When changing this, also update the layer separated collision matrix in PhysicsSystem.cpp.
 * and ensure that MaxPhysicsLayerCount in engine_limits.h is updated accordingly.
 * Also change GetPhysicsLayerAsIndex() and GetPhysicsLayerAsName() in physics_layer.h.
 */
enum class PhysicsLayer : uint32_t {
    None = 0u,
    Default = 1u << 0,      // Default layer for general objects.
    Player = 1u << 1,       // Layer for player characters.
    Enemy = 1u << 2,        // Layer for enemy characters.
    Projectile = 1u << 3,   // Layer for projectiles like bullets.
    Environment = 1u << 4,  // Layer for static environment objects.
    Trigger = 1u << 5,      // Layer for trigger volumes.
    UI = 1u << 6,           // Layer for UI elements.
    Layer0 = 1u << 7,       // Custom layer 0.
    Layer1 = 1u << 8,       // Custom layer 1.
    Layer2 = 1u << 9,       // Custom layer 2.
    Layer3 = 1u << 10,      // Custom layer 3.
    Layer4 = 1u << 11,      // Custom layer 4.
    Layer5 = 1u << 12,      // Custom layer 5.
    Layer6 = 1u << 13,      // Custom layer 6.
    Layer7 = 1u << 14,      // Custom layer 7.
    All = 0xFFFFFFFFu       // All layers.
};

constexpr inline PhysicsLayer operator|(PhysicsLayer a, PhysicsLayer b) {
    return static_cast<PhysicsLayer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr inline PhysicsLayer operator&(PhysicsLayer a, PhysicsLayer b) {
    return static_cast<PhysicsLayer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr inline PhysicsLayer& operator|=(PhysicsLayer& a, PhysicsLayer b) {
    a = a | b;
    return a;
}

constexpr inline PhysicsLayer& operator&=(PhysicsLayer& a, PhysicsLayer b) {
    a = a & b;
    return a;
}

constexpr inline PhysicsLayer operator~(PhysicsLayer a) {
    return static_cast<PhysicsLayer>(~static_cast<uint32_t>(a));
}

namespace Physics {

/** Returns the bit representation of the given PhysicsLayer. */
constexpr inline uint32_t ToBits(PhysicsLayer layer) {
    return static_cast<uint32_t>(layer);
}

/** Checks if the given category and mask have any overlapping bits set. */
constexpr inline bool HasLayer(PhysicsLayer mask, PhysicsLayer category) {
    return (ToBits(mask) & ToBits(category)) != 0;
}

/** Collision filter test. Collision can happen if this returns true. */
constexpr inline bool IsCollisionAllowed(PhysicsLayer categoryA, PhysicsLayer maskA, PhysicsLayer categoryB,
                                         PhysicsLayer maskB) {
    return HasLayer(maskA, categoryB) && HasLayer(maskB, categoryA);
}

/**
 * Converts a PhysicsLayer to its corresponding index (0-based) used in physics system.
 * This is used to put colliders into arrays based on their layer.
 */
constexpr inline uint8_t GetIndexFromPhysicsLayer(PhysicsLayer layer) {
    switch (layer) {
        case PhysicsLayer::None:
            return 0;
        case PhysicsLayer::Default:
            return 1;
        case PhysicsLayer::Player:
            return 2;
        case PhysicsLayer::Enemy:
            return 3;
        case PhysicsLayer::Projectile:
            return 4;
        case PhysicsLayer::Environment:
            return 5;
        case PhysicsLayer::Trigger:
            return 6;
        case PhysicsLayer::UI:
            return 7;
        case PhysicsLayer::Layer0:
            return 8;
        case PhysicsLayer::Layer1:
            return 9;
        case PhysicsLayer::Layer2:
            return 10;
        case PhysicsLayer::Layer3:
            return 11;
        case PhysicsLayer::Layer4:
            return 12;
        case PhysicsLayer::Layer5:
            return 13;
        case PhysicsLayer::Layer6:
            return 14;
        case PhysicsLayer::Layer7:
            return 15;
        default:
            return 0;
    }
}

/**
 * Converts a PhysicsLayer index (0-based) back to its corresponding PhysicsLayer.
 */
constexpr inline PhysicsLayer GetPhysicsLayerFromIndex(uint8_t index) {
    switch (index) {
        case 0:
            return PhysicsLayer::None;
        case 1:
            return PhysicsLayer::Default;
        case 2:
            return PhysicsLayer::Player;
        case 3:
            return PhysicsLayer::Enemy;
        case 4:
            return PhysicsLayer::Projectile;
        case 5:
            return PhysicsLayer::Environment;
        case 6:
            return PhysicsLayer::Trigger;
        case 7:
            return PhysicsLayer::UI;
        case 8:
            return PhysicsLayer::Layer0;
        case 9:
            return PhysicsLayer::Layer1;
        case 10:
            return PhysicsLayer::Layer2;
        case 11:
            return PhysicsLayer::Layer3;
        case 12:
            return PhysicsLayer::Layer4;
        case 13:
            return PhysicsLayer::Layer5;
        case 14:
            return PhysicsLayer::Layer6;
        case 15:
            return PhysicsLayer::Layer7;
        default:
            return PhysicsLayer::None;
    }
}

/** Converts a PhysicsLayer to its corresponding name as a string. */
constexpr inline const char* GetPhysicsLayerAsName(PhysicsLayer layer) {
    switch (layer) {
        case PhysicsLayer::None:
            return "None";
        case PhysicsLayer::Default:
            return "Default";
        case PhysicsLayer::Player:
            return "Player";
        case PhysicsLayer::Enemy:
            return "Enemy";
        case PhysicsLayer::Projectile:
            return "Projectile";
        case PhysicsLayer::Environment:
            return "Environment";
        case PhysicsLayer::Trigger:
            return "Trigger";
        case PhysicsLayer::UI:
            return "UI";
        case PhysicsLayer::Layer0:
            return "Layer0";
        case PhysicsLayer::Layer1:
            return "Layer1";
        case PhysicsLayer::Layer2:
            return "Layer2";
        case PhysicsLayer::Layer3:
            return "Layer3";
        case PhysicsLayer::Layer4:
            return "Layer4";
        case PhysicsLayer::Layer5:
            return "Layer5";
        case PhysicsLayer::Layer6:
            return "Layer6";
        case PhysicsLayer::Layer7:
            return "Layer7";
        default:
            return "Unknown";
    }
}

}  // namespace Physics
}  // namespace ME
