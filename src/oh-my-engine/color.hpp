#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <limits>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Color
{
  private:
    // WARN: RGBA memory layout is expected. Do not modify.
    //       It is required to cast color values to OpenGL-compatible pixel values.
    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
    uint8_t alpha_;

    explicit constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
        : red_(red),
          green_(green),
          blue_(blue),
          alpha_(alpha)
    {
    }

  public:
    Color() = default; // black

    // #region Static Factories

    static constexpr Color
    rgba(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
        return Color(red, green, blue, alpha);
    }

    static constexpr Color
    rgba(Vec4<uint8_t> v)
    {
        return Color(v[0], v[1], v[2], v[3]);
    }

    static constexpr Color
    rgba(float red, float green, float blue, float alpha)
    {
        auto cast = [](float x) { return static_cast<uint8_t>(x * 255.0f + 0.5f); };
        return Color(cast(red), cast(green), cast(blue), cast(alpha));
    }

    static constexpr Color
    rgba(Vec4f v)
    {
        return rgba(v[0], v[1], v[2], v[3]);
    }

    static constexpr Color
    rgb(uint8_t red, uint8_t green, uint8_t blue)
    {
        return rgba(red, green, blue, 255);
    }

    static constexpr Color
    rgb(int red, int green, int blue)
    {
        return rgb(uint8_t(red), uint8_t(green), uint8_t(blue));
    }

    static constexpr Color
    rgb(Vec3<uint8_t> v)
    {
        return Color(v[0], v[1], v[2], 255);
    }

    static constexpr Color
    rgb(Vec3f v)
    {
        return rgba(Vec4f{ v[0], v[1], v[2], 1.0f });
    }

    static constexpr Color
    rgb(float red, float green, float blue)
    {
        return rgba(red, green, blue, 1.0f);
    }

    static constexpr Color
    hex(uint32_t hex_value)
    {
        // Format: 0xRRGGBBAA  (alpha is ignored — fully opaque is assumed)
        uint8_t red   = ((hex_value >> 24) & 0xFF);
        uint8_t green = ((hex_value >> 16) & 0xFF);
        uint8_t blue  = ((hex_value >> 8) & 0xFF);

        return rgb(red, green, blue);
    }

    static constexpr Color
    hsv(float hue, float saturation, float value, float alpha = 1.0f)
    {
        auto  wrap = [](float x) { return x - std::floor(x / 360.0f) * 360.0f; };
        float h    = wrap(hue) / 60.0f;
        float c    = value * saturation;
        float x    = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
        float m    = value - c;

        float r, g, b;
        if (h < 1.0f)
        {
            r = c;
            g = x;
            b = 0.0f;
        }
        else if (h < 2.0f)
        {
            r = x;
            g = c;
            b = 0.0f;
        }
        else if (h < 3.0f)
        {
            r = 0.0f;
            g = c;
            b = x;
        }
        else if (h < 4.0f)
        {
            r = 0.0f;
            g = x;
            b = c;
        }
        else if (h < 5.0f)
        {
            r = x;
            g = 0.0f;
            b = c;
        }
        else
        {
            r = c;
            g = 0.0f;
            b = x;
        }

        return rgba(r + m, g + m, b + m, alpha);
    }

    static constexpr Color
    hsv(Vec3f v, float alpha = 1.0f)
    {
        return hsv(v[0], v[1], v[2], alpha);
    }

    // #endregion

    // #region Accessors

    Vec4<uint8_t>
    rgba() const
    {
        return { red_, green_, blue_, alpha_ };
    }

    Vec4f
    rgba_f() const
    {
        return Vec4f(rgba()) / max_channel_value;
    }

    Vec3<uint8_t>
    rgb() const
    {
        return { red_, green_, blue_ };
    }

    Vec3f
    rgb_f() const
    {
        return Vec3f(rgb()) / max_channel_value;
    }

    constexpr bool
    operator==(const Color &) const
        = default;

    // #endregion

    static constexpr uint max_channel_value = std::numeric_limits<decltype(alpha_)>::max();

    // #region Arithmetic operators

    friend Color
    operator+(const Color &lhs, const Color &rhs)
    {
        return Color::rgba(lhs.rgba_f() + rhs.rgba_f());
    }

    friend Color
    operator-(const Color &lhs, const Color &rhs)
    {
        return Color::rgba(lhs.rgba_f() - rhs.rgba_f());
    }

    friend Color
    operator*(const Color &lhs, float rhs)
    {
        return Color::rgba(lhs.rgba_f() * rhs);
    }

    friend Color
    operator*(float lhs, const Color &rhs)
    {
        return Color::rgba(rhs.rgba_f() * lhs);
    }

    friend Color
    operator/(const Color &lhs, float rhs)
    {
        return Color::rgba(lhs.rgba_f() / rhs);
    }

    // #endregion

    // #region Predefined pure colors

    static constexpr Color
    white()
    {
        return Color(255, 255, 255, 255);
    }

    static constexpr Color
    black()
    {
        return Color(0, 0, 0, 255);
    }

    static constexpr Color
    red()
    {
        return Color(255, 0, 0, 255);
    }

    static constexpr Color
    green()
    {
        return Color(0, 255, 0, 255);
    }

    static constexpr Color
    blue()
    {
        return Color(0, 0, 255, 255);
    }

    static constexpr Color
    magenta()
    {
        return Color(255, 0, 255, 255);
    }

    static constexpr Color
    cyan()
    {
        return Color(0, 255, 255, 255);
    }

    static constexpr Color
    yellow()
    {
        return Color(255, 255, 0, 255);
    }

    static constexpr Color
    transparent()
    {
        return Color(0, 0, 0, 0);
    }

    // #endregion
};

inline Color
grayscale(Color color)
{
    auto           rgb     = Vec3f(color.rgb());
    constexpr auto weights = Vec3f(0.299f, 0.587f, 0.114f);
    float          luma    = dot(rgb, weights);
    return Color::rgba(luma, luma, luma, float(color.rgba()[3]));
}

} // namespace ome

inline void
glColor(ome::Color color)
{
    auto [red, green, blue, alpha] = color.rgba_f();
    glColor4f(red, green, blue, alpha);
}
