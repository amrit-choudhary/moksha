#pragma once

/**
 * Utility function related to Random.
 */

#include <cmath>

#include "moksha/shared/math/math.h"
#include "moksha/shared/math/vec2.h"
#include "random_engine.h"

namespace ME {

namespace Utils {

inline ME::Vec2 RandomVec2OnCircle(ME::Random& rnd) {
    double angle = rnd.NextDouble() * ME::TAU;
    return ME::Vec2{static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
}

inline ME::Vec2 RandomVec2InCircle(ME::Random& rnd) {
    float radius = std::sqrt(rnd.NextDouble());
    double angle = rnd.NextDouble() * ME::TAU;
    return ME::Vec2{static_cast<float>(std::cos(angle)) * radius, static_cast<float>(std::sin(angle)) * radius};
}

}  // namespace Utils
}  // namespace ME
