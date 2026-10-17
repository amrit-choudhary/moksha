/**
 * Core function for random number generator.
 * This is the xoshiro128** 1.1 algorithm. It can generate 32 bit uint32 random number.
 * Reference: https://prng.di.unimi.it/xoshiro128starstar.c
 */
#pragma once

#include <cstdint>

namespace ME {

// Xoshiro128** RNG implementation.
class Random {
   public:
    // Default constructor.
    Random();

    // Constructor with seed.
    Random(uint32_t seed);

    /** Constructor with a seed string that gets converted to a uint32_t.
     * Can useTime to vary seed between runs.
     */
    Random(const char* seedString, bool useTime = false);

    /** Generate next random number in the range of 0 to uint32_t maximum.
     * Better use some of the helper function like range.
     */
    uint32_t Next();

    // Generate a random number in range [min, max].
    uint32_t NextRange(uint32_t min, uint32_t max);

    // Generates a double in range [0, 1).
    double NextDouble();

    // Generates a uint64_t.
    uint64_t Next64();

   private:
    uint32_t s[4];
    static inline uint32_t Rotl(const uint32_t x, int k) {
        return (x << k) | (x >> (32 - k));
    }
};

/**
 * Wighted Random engine.
 * Uses Random as base random engine.
 * Weight values are provided in lutValues
 * Expecting a 10 size luValues, this can represent any 10% value.
 * Example: [0, 0, 0, 1, 1, 2, 2, 2, 2, 3] means:
 * outcome 0 = 30% chance
 * outcome 1 = 20% chance
 * outcome 2 = 40% chane
 * outcome 3 = 10% chance
 */
class RandomWt {
   private:
    Random random;
    const uint32_t size = 10;
    uint8_t lut[10];

    RandomWt() = delete;                             // deleted default constructor.
    RandomWt(const RandomWt&) = delete;              // deleted copy constructor.
    RandomWt& operator=(const RandomWt&) = delete;   // deleted copy assignment.
    RandomWt(const RandomWt&&) = delete;             // deleted move constructor.
    RandomWt& operator=(const RandomWt&&) = delete;  // deleted move assignment.

   public:
    /**
     * @param seed = seed value that will feed underlying random generator.
     * @param lutValues = array of index of size 10. index is of weighted values.
     * Check RandomWt class comments for more info.
     */
    RandomWt(uint32_t seed, uint8_t* lutValues);
    ~RandomWt();

    // Get the next value based on the weighted chance values.
    uint8_t Next();
};

}  // namespace ME