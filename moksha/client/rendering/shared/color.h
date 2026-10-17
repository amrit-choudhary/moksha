/**
 * Color class definition.
 * This class represents a color in a 3D rendering engine.
 * It contains properties such as red, green, blue, and alpha values.
 * The color values are represented as floating-point numbers in the range [0.0, 1.0].
 */

#pragma once

namespace ME {

class Random;

class Color {
   public:
    constexpr Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

    constexpr Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {}

    /**
     * Constructor that takes a hexadecimal color string (e.g., "#FF5733").
     * The string should be in the format "RRGGBB" or "#RRGGBB".
     */
    Color(const char* hexColor);

    ~Color() = default;

    float r;
    float g;
    float b;
    float a;

    // Static constexpr common colors
    static constexpr Color White() {
        return Color(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static constexpr Color Black() {
        return Color(0.0f, 0.0f, 0.0f, 1.0f);
    }

    static constexpr Color Red() {
        return Color(1.0f, 0.0f, 0.0f, 1.0f);
    }

    static constexpr Color Green() {
        return Color(0.0f, 1.0f, 0.0f, 1.0f);
    }

    static constexpr Color Blue() {
        return Color(0.0f, 0.0f, 1.0f, 1.0f);
    }

    static constexpr Color Yellow() {
        return Color(1.0f, 1.0f, 0.0f, 1.0f);
    }

    static constexpr Color Cyan() {
        return Color(0.0f, 1.0f, 1.0f, 1.0f);
    }

    static constexpr Color Magenta() {
        return Color(1.0f, 0.0f, 1.0f, 1.0f);
    }

    static constexpr Color Orange() {
        return Color(1.0f, 0.5f, 0.0f, 1.0f);
    }

    static constexpr Color Purple() {
        return Color(0.5f, 0.0f, 0.5f, 1.0f);
    }

    static constexpr Color Pink() {
        return Color(1.0f, 0.75f, 0.8f, 1.0f);
    }

    static constexpr Color Brown() {
        return Color(0.6f, 0.3f, 0.0f, 1.0f);
    }

    static constexpr Color Gray() {
        return Color(0.5f, 0.5f, 0.5f, 1.0f);
    }

    static constexpr Color LightGray() {
        return Color(0.75f, 0.75f, 0.75f, 1.0f);
    }

    static constexpr Color DarkGray() {
        return Color(0.25f, 0.25f, 0.25f, 1.0f);
    }

    static constexpr Color Teal() {
        return Color(0.0f, 0.5f, 0.5f, 1.0f);
    }

    static constexpr Color Lime() {
        return Color(0.75f, 1.0f, 0.0f, 1.0f);
    }

    static constexpr Color Gold() {
        return Color(1.0f, 0.84f, 0.0f, 1.0f);
    }

    static constexpr Color Silver() {
        return Color(0.75f, 0.75f, 0.75f, 1.0f);
    }

    static constexpr Color Zero() {
        return Color(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // This uses it's own random seed that depends on time. Do not use in loop since it will
    // generate the same color as seed will be same. Use this for one off random color.
    static Color RandomColor();

    // Create a random color using the provided random engine.
    // This is useful for generating random colors in a loop or for specific use cases.
    static Color RandomColor(ME::Random& random);

    // Create a pretty random colors.
    static Color RandomColorPretty();

    // Create a pretty random color using the provided random engine.
    static Color RandomColorPretty(ME::Random& random);

    // Creates a ME::Color from HSV values.
    static ME::Color FromHSV(float h, float s, float v);

    // Converts one color channel from linear to gamma space.
    static float LinearToGamma(float linear);

    // Converts one color channel from gamma to linear space.
    static float GammaToLinear(float gamma);

    // Converts a color from linear to gamma space.
    static Color LinearToGamma(const Color& color);

    // Converts a color from gamma to linear space.
    static Color GammaToLinear(const Color& color);
};

}  // namespace ME
