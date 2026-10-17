/**
 * Tests for FP_24_8 fixed-point numbers (8 fractional bits, so 1.0 has raw value 256).
 */

#include "moksha/shared/math/fp_24_8.h"
#include "moksha/test_framework/test_framework.h"

using ME::FP_24_8;

TEST(FixedPoint, ConstructFromIntAndFloat) {
    EXPECT(FP_24_8(3).GetRaw() == 3 * 256);
    EXPECT(FP_24_8(1.5f).GetRaw() == 384);
    EXPECT(FP_24_8(-1.5f).GetRaw() == -384);
    EXPECT(FP_24_8::FromRawValue(128).ToFloat() == 0.5f);
}

TEST(FixedPoint, FloatRoundsToNearestStep) {
    // One step is 1/256 = 0.00390625; 0.6 of a step rounds up, 0.4 rounds down.
    EXPECT(FP_24_8(0.6f / 256.0f).GetRaw() == 1);
    EXPECT(FP_24_8(0.4f / 256.0f).GetRaw() == 0);
}

TEST(FixedPoint, AddSubtract) {
    FP_24_8 a(2.25f);
    FP_24_8 b(1.5f);

    EXPECT((a + b) == FP_24_8(3.75f));
    EXPECT((a - b) == FP_24_8(0.75f));
    EXPECT((-a) == FP_24_8(-2.25f));

    a += 1;
    EXPECT(a == FP_24_8(3.25f));
}

TEST(FixedPoint, MultiplyDivide) {
    EXPECT((FP_24_8(2.5f) * FP_24_8(4)) == FP_24_8(10));
    EXPECT((FP_24_8(-1.5f) * FP_24_8(2)) == FP_24_8(-3));
    EXPECT((FP_24_8(10) / FP_24_8(4)) == FP_24_8(2.5f));
    EXPECT((FP_24_8(1) / FP_24_8(3)).GetRaw() == 85);
}

TEST(FixedPoint, ToIntTruncatesTowardZero) {
    EXPECT(FP_24_8(2.75f).ToInt() == 2);
    EXPECT(FP_24_8(-2.75f).ToInt() == -2);
}

TEST(FixedPoint, Comparisons) {
    FP_24_8 small(1);
    FP_24_8 big(2);

    EXPECT(small < big);
    EXPECT(big > small);
    EXPECT(small <= small);
    EXPECT(big >= small);
    EXPECT(small != big);
}
