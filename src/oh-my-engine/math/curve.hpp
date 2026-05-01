#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

namespace math {

template <std::size_t Dimension = 1, float DomainMin = 0.0f, float DomainMax = 1.0f>
    requires(DomainMin < DomainMax)
class ParametricCurve
{
  private:
    using Vector_ = math::Vector<Dimension>;

    std::function<Vector_(float)> function_;

  public:
    ParametricCurve(std::function<Vector_(float)> function)
        : function_(std::move(function))
    {
    }

    Vector_
    operator()(float t) const
    {
        return function_(std::clamp(t, DomainMin, DomainMax));
    }

    static ParametricCurve
    linear()
    {
        return { [](float t) { return (t - DomainMin) / (DomainMax - DomainMin); } };
    }

    static constexpr ParametricCurve
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

}; // namespace math

using Curve      = math::ParametricCurve<1>;
using PlaneCurve = math::ParametricCurve<2>;
using SpaceCurve = math::ParametricCurve<3>;
} // namespace ome
