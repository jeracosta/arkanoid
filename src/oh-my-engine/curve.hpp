#pragma once

#include <memory>
namespace ome {

template <typename T>
class Curve
{
  public:
    virtual T
    operator()(float t) const
        = 0;
    virtual ~Curve() = default;
};

template <typename T>
class CurveProcess
{
    std::shared_ptr<Curve<T>> curve_;
    float                     speed_    = 1.0f;
    float                     progress_ = 0.0f;
    bool                      reversed_ = false;

  public:
    explicit CurveProcess(std::shared_ptr<Curve<T>> curve, float speed = 1.0f)
        : curve_(std::move(curve)),
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

    void
    complete()
    {
        progress_ = 1.0f;
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
        reversed_ = !reversed_;
        restart();
    }

    bool
    is_reversed() const
    {
        return reversed_;
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

} // namespace ome
