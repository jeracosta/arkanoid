#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>
#include <variant>

namespace ome {

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

        float value = std::visit([t](auto const &curve) { return curve(t); }, curve_);
        assert(value >= codomain.min && value <= codomain.max);

        return value;
    }
};

template <typename T>
class Interpolation
{
  protected:
    T           from_;
    T           to_;
    EasingCurve curve_;

  public:
    Interpolation(T from, T to, EasingCurve curve)
        : from_(std::move(from)),
          to_(std::move(to)),
          curve_(std::move(curve))
    {
    }

    T
    operator()(float progress) const
    {
        float u = curve_(progress);
        return from_ * (1.0f - u) + to_ * u;
    }

    class Process;
};

template <typename T>
class Interpolation<T>::Process : private Interpolation<T>
{
  private:
    float speed_    = 1.0f;
    float progress_ = 0.0f;
    bool  reversed_ = false;

  public:
    Process(T from, T to, EasingCurve curve, float speed = 1.0f)
        : Interpolation<T>(std::move(from), std::move(to), std::move(curve)),
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
        std::swap(this->from_, this->to_);
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
        return reversed_ ? to_ : from_;
    }

    void
    from(T new_from)
    {
        auto &target = is_reversed() ? to_ : from_;
        target       = std::move(new_from);
    }

    const T &
    to() const
    {
        return reversed_ ? from_ : to_;
    }

    void
    to(T new_to)
    {
        auto &target = is_reversed() ? from_ : to_;
        target       = std::move(new_to);
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
        return (*this)(progress_);
    }
};

} // namespace ome
