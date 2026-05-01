#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
namespace ome {

class EasingCurve
{
  public:
    using Output = float;

    // NOTE: This kind of curve maps [0, 1] to [0, 1].
    //       They define the form of an interpolation graph, not the range of values it can take.

    struct Domain
    {
        float min;
        float max;
    };

    struct Codomain
    {
        float min;
        float max;
    };

    static constexpr Domain   domain   = { 0, 1 };
    static constexpr Codomain codomain = { 0, 1 };

  private:
    std::function<Output(float)> function_;

  public:
    EasingCurve(std::function<Output(float)> function)
        : function_(std::move(function))
    {
    }

    Output
    operator()(float t) const
    {
        auto value = function_(std::clamp(t, domain.min, domain.max));
        assert(value == std::clamp(value, codomain.min, codomain.max));
        return value;
    }

    static EasingCurve
    linear()
    {
        return { [](float t) { return (t - domain.min) / (domain.max - domain.min); } };
    }

    static EasingCurve
    smoothstep(float steepness = 1.0f)
    {
        assert(steepness > 0.0f);
        return { [steepness](float t)
        {
            float x = (t - domain.min) / (domain.max - domain.min);
            x       = std::clamp(x, 0.0f, 1.0f);

            float a = std::pow(x, steepness);
            float b = std::pow(1.0f - x, steepness);

            float s = a / (a + b);

            return s * s * (3.0f - 2.0f * s);
        } };
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
        : from_(from),
          to_(to),
          curve_(std::move(curve))
    {
    }

    T
    operator()(float progress) const
    {
        auto mapped_progress = curve_(progress);
        return to_ * mapped_progress + from_ * (1.0f - mapped_progress);
    }

    class Process;
};

template <typename T>
class Interpolation<T>::Process : private Interpolation<T>
{
  private:
    float speed_;
    float progress_ = 0.0f;
    bool  flipped_  = false;

  public:
    Process(T from, T to, EasingCurve curve, float speed = 1.0f)
        : Interpolation<T>(from, to, std::move(curve)),
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
    flip()
    {
        std::swap(this->from_, this->to_);
        flipped_ = !flipped_;
        restart();
    }

    bool
    is_foward() const
    {
        return !flipped_;
    }

    bool
    is_backward() const
    {
        return flipped_;
    }

    void
    forward_endpoints(T new_from, T new_to)
    {
        this->from_ = new_from;
        this->to_   = new_to;

        if (flipped_)
        {
            std::swap(this->from_, this->to_);
        }
    }

    void
    update(float delta_time)
    {
        progress_ += speed_ * delta_time;
        progress_ = std::clamp(progress_, 0.0f, 1.0f);
    }
};

} // namespace ome
