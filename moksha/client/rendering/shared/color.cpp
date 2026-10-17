#include "color.h"

#include <math.h>

#include <cstdlib>

#include "moksha/shared/math/math.h"
#include "moksha/shared/random/random_engine.h"

ME::Color::Color(const char* hexColor) {
    if (hexColor[0] == '#') {
        hexColor++;
    }

    char rBuf[3] = {hexColor[0], hexColor[1], '\0'};
    char gBuf[3] = {hexColor[2], hexColor[3], '\0'};
    char bBuf[3] = {hexColor[4], hexColor[5], '\0'};

    r = static_cast<float>(strtol(rBuf, nullptr, 16)) / 255.0f;
    g = static_cast<float>(strtol(gBuf, nullptr, 16)) / 255.0f;
    b = static_cast<float>(strtol(bBuf, nullptr, 16)) / 255.0f;
    a = 1.0f;  // Default alpha value

    // Hex values are expected to be in sRGB space, so we convert them to linear space.
    r = GammaToLinear(r);
    g = GammaToLinear(g);
    b = GammaToLinear(b);

    // Ensure color values are clamped between 0.0 and 1.0
    r = ME::Math::Clamp(r, 0.0f, 1.0f);
    g = ME::Math::Clamp(g, 0.0f, 1.0f);
    b = ME::Math::Clamp(b, 0.0f, 1.0f);
}

ME::Color ME::Color::RandomColor() {
    ME::Random random;
    float r = random.NextDouble();
    float g = random.NextDouble();
    float b = random.NextDouble();
    float a = 1.0f;
    return Color{r, g, b, a};
}

ME::Color ME::Color::RandomColor(ME::Random& random) {
    float r = random.NextDouble();
    float g = random.NextDouble();
    float b = random.NextDouble();
    float a = 1.0f;
    return Color{r, g, b, a};
}

ME::Color ME::Color::RandomColorPretty() {
    ME::Random random;
    float h = random.NextDouble();
    float s = 0.7f + 0.3f * random.NextDouble();
    float v = 0.6f + 0.4f * random.NextDouble();
    return FromHSV(h, s, v);
}

ME::Color ME::Color::RandomColorPretty(ME::Random& random) {
    float h = random.NextDouble();
    float s = 0.7f + 0.3f * random.NextDouble();
    float v = 0.6f + 0.4f * random.NextDouble();
    return FromHSV(h, s, v);
}

ME::Color ME::Color::FromHSV(float h, float s, float v) {
    float r, g, b;
    int i = int(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (i % 6) {
        case 0:
            r = v, g = t, b = p;
            break;
        case 1:
            r = q, g = v, b = p;
            break;
        case 2:
            r = p, g = v, b = t;
            break;
        case 3:
            r = p, g = q, b = v;
            break;
        case 4:
            r = t, g = p, b = v;
            break;
        case 5:
            r = v, g = p, b = q;
            break;
    }

    return ME::Color(r, g, b, 1.0f);
}

// Converts a single channel from linear to sRGB gamma space
float ME::Color::LinearToGamma(float linear) {
    if (linear <= 0.0031308f)
        return 12.92f * linear;
    else
        return 1.055f * powf(linear, 1.0f / 2.4f) - 0.055f;
}

// Converts a single channel from sRGB gamma space to linear
float ME::Color::GammaToLinear(float gamma) {
    if (gamma <= 0.04045f)
        return gamma / 12.92f;
    else
        return powf((gamma + 0.055f) / 1.055f, 2.4f);
}

ME::Color ME::Color::LinearToGamma(const Color& color) {
    return Color(LinearToGamma(color.r), LinearToGamma(color.g), LinearToGamma(color.b), color.a);
}

ME::Color ME::Color::GammaToLinear(const Color& color) {
    return Color(GammaToLinear(color.r), GammaToLinear(color.g), GammaToLinear(color.b), color.a);
}
