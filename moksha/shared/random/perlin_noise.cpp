#include "perlin_noise.h"

#include "stb_perlin.h"

ME::PerlinNoise::PerlinNoise(Random& rnd) : PerlinNoise(rnd.Next()) {}

ME::PerlinNoise::PerlinNoise(uint32_t s) {
    seed = static_cast<uint8_t>(s & 0xFF);
    // Large, well-separated offsets so different seeds sample unrelated regions
    // of the (period-256) noise field.
    offsetX = static_cast<float>((s >> 8) & 0xFFFF) * 17.0f;
    offsetY = static_cast<float>((s >> 16) & 0xFFFF) * 31.0f;
    offsetZ = static_cast<float>((s >> 24) & 0xFF) * 53.0f;
}

float ME::PerlinNoise::Noise(float x, float y, float z, int xWrap, int yWrap, int zWrap) const {
    return stb_perlin_noise3_seed(x, y, z, xWrap, yWrap, zWrap, seed);
}

float ME::PerlinNoise::Noise(const Vec2& p) const {
    return Noise(p.x, p.y);
}

float ME::PerlinNoise::Fbm(float x, float y, float z, float lacunarity, float gain, int octaves) const {
    return stb_perlin_fbm_noise3(x + offsetX, y + offsetY, z + offsetZ, lacunarity, gain, octaves);
}

float ME::PerlinNoise::Fbm(const Vec2& p, float lacunarity, float gain, int octaves) const {
    return Fbm(p.x, p.y, 0.0f, lacunarity, gain, octaves);
}

float ME::PerlinNoise::FbmMaxAmplitude(float gain, int octaves) {
    float amplitude = 1.0f;
    float sum = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        sum += amplitude;
        amplitude *= gain;
    }
    return sum;
}

float ME::PerlinNoise::Ridge(float x, float y, float z, float lacunarity, float gain, float offset, int octaves) const {
    return stb_perlin_ridge_noise3(x + offsetX, y + offsetY, z + offsetZ, lacunarity, gain, offset, octaves);
}

float ME::PerlinNoise::Turbulence(float x, float y, float z, float lacunarity, float gain, int octaves) const {
    return stb_perlin_turbulence_noise3(x + offsetX, y + offsetY, z + offsetZ, lacunarity, gain, octaves);
}
