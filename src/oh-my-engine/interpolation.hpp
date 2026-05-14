#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <variant>

#include "oh-my-engine/curve.hpp"

namespace ome {

// #region Easing curve

// Maps [0, 1] to [0, 1] to describe the form of an interpolation graph.
class EasingCurve
{
  public:
    struct Domain
    {
        float min;
        float max;
    };
    static constexpr Domain domain = { 0.0f, 1.0f };

    struct Codomain
    {
        float min;
        float max;
    };
    static constexpr Codomain codomain = { 0.0f, 1.0f };

    using Output = float;

  private:
    struct Linear_
    {
        float
        operator()(float t) const noexcept
        {
            return t;
        }
    };

    struct Smoothstep_
    {
        float steepness = 1.0f;

        float
        operator()(float t) const
        {
            assert(steepness > 0.0f);

            float x = std::clamp(t, 0.0f, 1.0f);

            float a = std::pow(x, steepness);
            float b = std::pow(1.0f - x, steepness);

            float s = a / (a + b);
            return s * s * (3.0f - 2.0f * s);
        }
    };

    using Variant_ = std::variant<Linear_, Smoothstep_>;

    Variant_ curve_ = Linear_{};

  public:
    EasingCurve() = default;

    EasingCurve(auto curve)
        : curve_(curve)
    {
    }

    static EasingCurve
    linear()
    {
        return Linear_{};
    }

    static EasingCurve
    smoothstep(float steepness = 1.0f)
    {
        return Smoothstep_{ steepness };
    }

    Output
    operator()(float t) const
    {
        t = std::clamp(t, domain.min, domain.max);

        return std::visit([t](auto const &curve) { return curve(t); }, curve_);
    }
};

// #endregion

// #region Interpolation curve

template <typename T>
class Interpolation : public Curve<T>
{
    T           from_;
    T           to_;
    EasingCurve easing_;

  public:
    Interpolation() = default;

    Interpolation(T from, T to, EasingCurve easing = EasingCurve::linear())
        : from_(std::move(from)),
          to_(std::move(to)),
          easing_(std::move(easing))
    {
    }

    T
    operator()(float t) const override
    {
        float u = easing_(t);
        return from_ * (1.0f - u) + to_ * u;
    }

    T &
    from()
    {
        return from_;
    }

    const T &
    from() const
    {
        return from_;
    }

    void
    from(T new_from)
    {
        from_ = std::move(new_from);
    }

    T &
    to()
    {
        return to_;
    }

    const T &
    to() const
    {
        return to_;
    }

    void
    to(T new_to)
    {
        to_ = std::move(new_to);
    }
};

// #endregion

// #region Utilities

constexpr auto
lerp(const auto &from, const auto &to, float t)
{
    return Interpolation{ from, to, EasingCurve::linear() }(t);
}

// #endregion

} // namespace ome
