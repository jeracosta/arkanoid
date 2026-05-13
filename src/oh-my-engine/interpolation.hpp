#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace ome {

// #region Easing curves

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

// #region Curve<T>

template <typename T>
class Curve
{
  public:
    virtual T operator()(float t) const = 0;
    virtual ~Curve() = default;
};

// #endregion

// #region Interpolation curve

template <typename T>
class InterpolationCurve : public Curve<T>
{
    T           from_;
    T           to_;
    EasingCurve easing_;

  public:
    InterpolationCurve() = default;

    InterpolationCurve(T from, T to, EasingCurve easing = EasingCurve::linear())
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

// #region Spline curve

template <typename T>
struct SplineKey
{
    float position;
    T     value;
};

template <typename T>
struct SplineKnot
{
    float position;
    T     value;
    T     tangent;
};

template <typename T>
class SplineCurve : public Curve<T>
{
    std::vector<SplineKnot<T>> knots_;

    static T
    segment_(const SplineKnot<T> &a, const SplineKnot<T> &b, float t)
    {
        float dt = b.position - a.position;
        float s  = (t - a.position) / dt;
        float s2 = s * s;
        float s3 = s2 * s;

        float h00 = 2.0f * s3 - 3.0f * s2 + 1.0f;
        float h10 = s3 - 2.0f * s2 + s;
        float h01 = -2.0f * s3 + 3.0f * s2;
        float h11 = s3 - s2;

        return a.value * h00 + a.tangent * (dt * h10) + b.value * h01 + b.tangent * (dt * h11);
    }

  public:
    SplineCurve() = default;

    explicit SplineCurve(std::vector<SplineKnot<T>> knots)
        : knots_(std::move(knots))
    {
    }

    T
    operator()(float t) const override
    {
        if (knots_.empty())
            return T{};

        if (t <= knots_.front().position)
            return knots_.front().value;

        if (t >= knots_.back().position)
            return knots_.back().value;

        auto it = std::ranges::upper_bound(knots_, t, {}, &SplineKnot<T>::position);

        return segment_(*(it - 1), *it, t);
    }

    static std::vector<SplineKnot<T>>
    catmull_rom(std::initializer_list<SplineKey<T>> keys)
    {
        auto n      = keys.size();
        auto *begin = keys.begin();
        auto knots  = std::vector<SplineKnot<T>>();
        knots.reserve(n);

        auto tangent = [&](std::size_t i) -> T
        {
            if (n == 1)
                return T{};

            if (i == 0)
                return (begin[1].value - begin[0].value) / (begin[1].position - begin[0].position);

            if (i == n - 1)
                return (begin[i].value - begin[i - 1].value)
                       / (begin[i].position - begin[i - 1].position);

            return (begin[i + 1].value - begin[i - 1].value)
                   / (begin[i + 1].position - begin[i - 1].position);
        };

        for (std::size_t i = 0; i < n; ++i)
        {
            knots.push_back({ .position = begin[i].position,
                              .value    = begin[i].value,
                              .tangent  = tangent(i) });
        }
        return knots;
    }
};

// #endregion

// #region Curve process

template <typename T>
class CurveProcess
{
    std::shared_ptr<Curve<T>> curve_;
    float                     speed_    = 1.0f;
    float                     progress_ = 0.0f;
    bool                      reversed_ = false;

    InterpolationCurve<T> *
    interpolation_()
    {
        return dynamic_cast<InterpolationCurve<T> *>(curve_.get());
    }

    const InterpolationCurve<T> *
    interpolation_() const
    {
        return dynamic_cast<const InterpolationCurve<T> *>(curve_.get());
    }

  public:
    explicit CurveProcess(std::shared_ptr<Curve<T>> curve, float speed = 1.0f)
        : curve_(std::move(curve)),
          speed_(speed)
    {
    }

    CurveProcess(T from, T to, EasingCurve easing, float speed = 1.0f)
        : curve_(std::make_shared<InterpolationCurve<T>>(std::move(from), std::move(to), std::move(easing))),
          speed_(speed)
    {
    }

    bool
    is_completed() const
    {
        return progress_ >= 1.0f;
    }

    void
    restart()
    {
        progress_ = 0.0f;
    }

    float
    speed() const
    {
        return speed_;
    }

    void
    set_speed(float new_speed)
    {
        speed_ = new_speed;
    }

    void
    reverse()
    {
        if (auto *ic = interpolation_())
        {
            std::swap(ic->from(), ic->to());
        }
        reversed_ = !reversed_;
        restart();
    }

    bool
    is_reversed() const
    {
        return reversed_;
    }

    const T &
    from() const
    {
        static const T default_{};
        if (auto *ic = interpolation_())
        {
            return reversed_ ? ic->to() : ic->from();
        }
        return default_;
    }

    void
    from(T new_from)
    {
        if (auto *ic = interpolation_())
        {
            (reversed_ ? ic->to() : ic->from()) = std::move(new_from);
        }
    }

    const T &
    to() const
    {
        static const T default_{};
        if (auto *ic = interpolation_())
        {
            return reversed_ ? ic->from() : ic->to();
        }
        return default_;
    }

    void
    to(T new_to)
    {
        if (auto *ic = interpolation_())
        {
            (reversed_ ? ic->from() : ic->to()) = std::move(new_to);
        }
    }

    float
    progress() const
    {
        return progress_;
    }

    void
    update(float delta_time)
    {
        progress_ = std::clamp(progress_ + speed_ * delta_time, 0.0f, 1.0f);
    }

    T
    value() const
    {
        float t = reversed_ ? 1.0f - progress_ : progress_;
        return (*curve_)(t);
    }
};

// #endregion

// #region Utilities

constexpr auto
lerp(const auto &from, const auto &to, float t)
{
    return InterpolationCurve{ from, to, EasingCurve::linear() }(t);
}

// #endregion

} // namespace ome
