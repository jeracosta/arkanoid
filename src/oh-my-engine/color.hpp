#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <stdexcept>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Color
{
  private:
    Vec4f rgba_{};

    // Private: values already range-checked by the static factory
    explicit constexpr Color(float r, float g, float b, float a)
        : rgba_{ r, g, b, a }
    {
    }

  public:
    Color() = default; // black

    // #region Static Factories

    static Color
    rgba(float r, float g, float b, float a)
    {
        if (r < 0.0f || r > 1.0f)
            throw std::domain_error("Color red   out of range [0, 1]");
        if (g < 0.0f || g > 1.0f)
            throw std::domain_error("Color green out of range [0, 1]");
        if (b < 0.0f || b > 1.0f)
            throw std::domain_error("Color blue  out of range [0, 1]");
        if (a < 0.0f || a > 1.0f)
            throw std::domain_error("Color alpha out of range [0, 1]");
        return Color(r, g, b, a);
    }

    static Color
    rgb(float r, float g, float b)
    {
        return rgba(r, g, b, 1.0f);
    }

    static Color
    rgba(int r, int g, int b, int a)
    {
        if (r < 0 || r > 255)
            throw std::domain_error("Color red   out of range [0, 255]");
        if (g < 0 || g > 255)
            throw std::domain_error("Color green out of range [0, 255]");
        if (b < 0 || b > 255)
            throw std::domain_error("Color blue  out of range [0, 255]");
        if (a < 0 || a > 255)
            throw std::domain_error("Color alpha out of range [0, 255]");
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    static Color
    rgb(int r, int g, int b)
    {
        return rgba(r, g, b, 255);
    }

    // #endregion

    // #region Accessors

    Vec4f
    rgba() const
    {
        return rgba_;
    }

    float
    red() const
    {
        return rgba_[0];
    }

    float
    green() const
    {
        return rgba_[1];
    }

    float
    blue() const
    {
        return rgba_[2];
    }

    float
    alpha() const
    {
        return rgba_[3];
    }

    // #endregion
};

inline Color
grayscale(Color color)
{
    auto  c    = color.rgba();
    float luma = 0.299f * c[0] + 0.587f * c[1] + 0.114f * c[2];
    return Color::rgb(luma, luma, luma);
}

} // namespace ome

inline void
glColor(ome::Color color)
{
    glColor3f(color.red(), color.green(), color.blue());
}
