/**
 * Tests for the Random (xoshiro128**) and RandomWt generators.
 */

#include <cstdint>

#include "moksha/shared/random/random_engine.h"
#include "moksha/test_framework/test_framework.h"

static constexpr int SAMPLES = 1000;

TEST(Random, SameSeedSameSequence) {
    ME::Random a(1234);
    ME::Random b(1234);

    for (int i = 0; i < SAMPLES; ++i) {
        ASSERT(a.Next() == b.Next());
    }
}

TEST(Random, DifferentSeedsDiffer) {
    ME::Random a(1);
    ME::Random b(2);
    int matches = 0;

    for (int i = 0; i < SAMPLES; ++i) {
        if (a.Next() == b.Next()) {
            matches++;
        }
    }
    EXPECT(matches < 5);
}

TEST(Random, SeedStringIsDeterministic) {
    ME::Random a("moksha");
    ME::Random b("moksha");

    EXPECT(a.Next() == b.Next());
}

TEST(Random, NextRangeIsInclusiveAndInBounds) {
    ME::Random rnd(42);
    bool sawMin = false;
    bool sawMax = false;

    for (int i = 0; i < SAMPLES; ++i) {
        uint32_t value = rnd.NextRange(3, 6);
        ASSERT(value >= 3 && value <= 6);
        sawMin = sawMin || value == 3;
        sawMax = sawMax || value == 6;
    }
    EXPECT(sawMin);
    EXPECT(sawMax);
}

TEST(Random, NextDoubleInUnitRange) {
    ME::Random rnd(42);

    for (int i = 0; i < SAMPLES; ++i) {
        double value = rnd.NextDouble();
        ASSERT(value >= 0.0 && value < 1.0);
    }
}

TEST(RandomWt, OnlyReturnsTableValues) {
    uint8_t lut[10] = {0, 0, 0, 1, 1, 2, 2, 2, 2, 3};
    ME::RandomWt rnd(7, lut);
    int counts[4] = {};

    for (int i = 0; i < SAMPLES; ++i) {
        uint8_t value = rnd.Next();
        ASSERT(value <= 3);
        counts[value]++;
    }
    // Weights 30/20/40/10%: every outcome appears, and 2 (40%) beats 3 (10%).
    EXPECT(counts[0] > 0 && counts[1] > 0 && counts[2] > 0 && counts[3] > 0);
    EXPECT(counts[2] > counts[3]);
}
