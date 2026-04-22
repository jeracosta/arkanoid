#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

namespace ome {

class Curve
{
  private:
    std::function<float(float)> function_;

  public:
    // function should map [0, 1] to [0, 1]
    Curve(std::function<float(float)> function)
        : function_(std::move(function))
    {
    }

    float
    operator()(float t) const
    {
        return function_(t);
    }

    static constexpr Curve
    linear()
    {
        return { [](float t) { return t; } };
    }

    static constexpr Curve
    smoothstep(float steepness = 1.0f)
    {
        return { [steepness](float t)
        {
            float x = std::clamp(t, 0.0f, 1.0f);

            float a = std::pow(x, steepness);
            float b = std::pow(1.0f - x, steepness);

            float s = a / (a + b);

            return s * s * (3.0f - 2.0f * s);
        } };
    }
};

} // namespace ome
