#pragma once

#include <algorithm>
#include <functional>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

namespace math {

template <class T>
struct ParametricCurve
{
  public:
    struct Domain
    {
        float from = 0.0f;
        float to   = 1.0f;
    };

    std::function<T(float, Domain)> function;

    Domain domain;

    T
    operator()(float t) const
    {
        auto clamped = rising() ? std::clamp(t, domain.to, domain.from)
                                : std::clamp(t, domain.from, domain.to);

        return function(clamped, domain);
    }

    void
    flip_domain()
    {
        std::swap(domain.from, domain.to);
    }

    bool
    rising() const
    {
        return domain.to > domain.from;
    }

    static ParametricCurve
    line(Domain domain)
    {
        auto &&function
            = [](float t, Domain domain) { return (t - domain.from) / (domain.to - domain.from); };

        return { function, domain };
    }

    static ParametricCurve
    smoothstep(Domain domain, float steepness = 1.0f)
    {
        auto &&function = [steepness](float t, Domain domain)
        {
            const float denom = (domain.to - domain.from);
            const float x     = denom != 0.0f ? (t - domain.from) / denom : 0.0f;

            const float clamped = std::clamp(x, 0.0f, 1.0f);

            const float k = std::max(1.0f, steepness);
            const float s = std::pow(clamped, k);

            const float smooth = s * s * (3.0f - 2.0f * s);

            return T(smooth);
        };

        return { function, domain };
    }
};

}; // namespace math

using Curve      = math::ParametricCurve<float>;
using PlaneCurve = math::ParametricCurve<Vec2f>;
using SpaceCurve = math::ParametricCurve<Vec3f>;
} // namespace ome
