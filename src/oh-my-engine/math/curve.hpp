#pragma once

#include <algorithm>
#include <cmath>
#include <functional>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

namespace math {

template <class TOutput = float, float TDomainMin = 0.0f, float TDomainMax = 1.0f>
    requires(TDomainMin < TDomainMax)
class EasingCurve
{
  private:
    std::function<TOutput(float)> function_;

  public:
    EasingCurve(std::function<TOutput(float)> function)
        : function_(std::move(function))
    {
    }

    TOutput
    operator()(float t) const
    {
        return function_(std::clamp(t, TDomainMin, TDomainMax));
    }

    static EasingCurve
    linear()
    {
        return { [](float t) { return (t - TDomainMin) / (TDomainMax - TDomainMin); } };
    }

    static constexpr EasingCurve
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

} // namespace ome
