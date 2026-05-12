#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
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

    static Color
    rgba(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
        return Color(red, green, blue, alpha);
    }

    static Color
    rgb(uint8_t red, uint8_t green, uint8_t blue)
    {
        return rgba(red, green, blue, 255);
    }

    static Color
    hex(uint32_t hex_value)
    {
        uint8_t red   = ((hex_value >> 16) & 0xFF);
        uint8_t green = ((hex_value >> 8) & 0xFF);
        uint8_t blue  = ((hex_value) & 0xFF);

        return rgb(red, green, blue);
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

    // #endregion

    static constexpr uint max_channel_value = std::numeric_limits<decltype(alpha_)>::max();
};

inline Color
grayscale(Color color)
{
    auto           rgb     = Vec3f(color.rgb());
    constexpr auto weights = Vec3f(0.299f, 0.587f, 0.114f);
    float          luma    = dot(rgb, weights);
    return Color::rgba(luma, luma, luma, color.rgba()[3]);
}

} // namespace ome

inline void
glColor(ome::Color color)
{
    auto [red, green, blue, alpha] = color.rgba_f();
    glColor4f(red, green, blue, alpha);
}
