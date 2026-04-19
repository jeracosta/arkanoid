#pragma once

#include <chrono>

#include "oh-my-engine/chronometer.hpp"

namespace ome {

namespace game {
class Session; // forward declaration
}

// forward declarations
class Pause;

class Time
{
  public:
    using Unit = std::chrono::duration<float, std::ratio<1>>; // seconds as float

  private:
    Chronometer<Unit>          chronometer_;
    Chronometer<Unit>::Reading current_time_;
    float                      unfrozen_time_scale_ = 1;

    bool
    frozen_() const
    {
        return chronometer_.scale() == 0;
    }

    void
    freeze_()
    {
        chronometer_.scale(0);
    }

    void
    unfreeze_()
    {
        chronometer_.scale(unfrozen_time_scale_);
    }

    friend class Pause;

  public:
    float
    elapsed() const
    {
        return current_time_.elapsed;
    }

    float
    since(const auto &time_point) const
    {
        auto now = decltype(chronometer_)::Clock::now();
        return std::chrono::duration<float>(now - time_point).count();
    }

    float
    delta() const
    {
        return current_time_.delta;
    }

    float
    scale() const
    {
        return unfrozen_time_scale_;
    }

    void
    scale(float new_value)
    {
        chronometer_.scale(new_value);
        unfrozen_time_scale_ = new_value;
    }

    class
    {
      private:
        Chronometer<Unit>          chronometer_;
        Chronometer<Unit>::Reading current_time_;

      public:
        float
        elapsed() const
        {
            return current_time_.elapsed;
        }

        float
        delta() const
        {
            return current_time_.delta;
        }

      private:
        void
        update_()
        {
            current_time_ = chronometer_.read();
        }

        friend class Time;

    } unscaled;

  private:
    void
    update_()
    {
        current_time_ = chronometer_.read();
        unscaled.update_();
    }

    friend class Game;
};

} // namespace ome
