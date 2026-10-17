#pragma once

#include <cstdint>

#include "moksha/shared/math/vec2.h"
#include "random_engine.h"

namespace ME {

// Wraps vendored stb_perlin (moksha/shared/random/stb_perlin.h) with ME::Random-derived seeding.
class PerlinNoise {
   public:
    // Draws a seed from rnd so noise fields are reproducible from the same Random stream.
    explicit PerlinNoise(Random& rnd);
    explicit PerlinNoise(uint32_t seed);

    // Single-octave noise, roughly in [-1, 1]. wrap must be 0 or a power of two.
    float Noise(float x, float y, float z = 0.0f, int xWrap = 0, int yWrap = 0, int zWrap = 0) const;
    float Noise(const Vec2& p) const;

    // Fractal Brownian motion (heightmap-style terrain noise).
    float Fbm(float x, float y, float z, float lacunarity, float gain, int octaves) const;
    float Fbm(const Vec2& p, float lacunarity, float gain, int octaves) const;

    // Analytic max magnitude of Fbm's output (sum of per-octave amplitudes); divide by this
    // to normalize Fbm's result into [-1, 1]. Depends only on gain/octaves, not lacunarity.
    static float FbmMaxAmplitude(float gain, int octaves);

    // Ridged multifractal (sharp ridges - mountains).
    float Ridge(float x, float y, float z, float lacunarity, float gain, float offset, int octaves) const;

    // Turbulence (absolute-value fbm - billowy/marble).
    float Turbulence(float x, float y, float z, float lacunarity, float gain, int octaves) const;

   private:
    // stb_perlin_noise3_seed's seed byte, used directly by Noise().
    uint8_t seed;
    // Coordinate offset derived from the same seed, applied before calling into
    // fbm/ridge/turbulence, which hardcode their own per-octave seeds (0..octaves-1)
    // and otherwise can't be varied per-instance.
    float offsetX;
    float offsetY;
    float offsetZ;
};

}  // namespace ME
