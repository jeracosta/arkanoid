#pragma once

#include "interpolation.hpp"

namespace ome {

template <typename T>
class Interpolator
{
  public:
    struct Callbacks
    {
        std::function<void(T)> on_updated  = {};
        std::function<void(T)> on_finished = {};
    };

  private:
    Interpolation<T> interpolation_;
    Callbacks        callbacks_;
    float            progress_ = 0;
    float            speed_    = 1;

  public:
    Interpolator(Interpolation<T> interpolation, Callbacks callbacks = {})
        : interpolation_(std::move(interpolation)),
          callbacks_(std::move(callbacks))
    {
    }

    bool
    finished() const
    {
        return progress_ >= 1;
    }

    void
    update(float delta)
    {
        progress_ += delta;
        progress_ = std::clamp(progress_, 0.0f, 1.0f);

        if (callbacks_.on_updated_)
        {
            on_updated_(interpolation_(progress_));
        }

        if (finished() && callbacks_.on_finished_)
        {
            on_finished_(interpolation_(progress_));
        }
    }

    void
    reset()
    {
        progress_ = 0.0f;
    }

    void
    flip()
    {
        interpolation_.flip();
        progress_ = 1.0f - progress_;
    }

    const Interpolation<T> &
    interpolation() const
    {
        return interpolation_;
    }
};

} // namespace ome
