#pragma once
#include <chrono>

namespace ome {

template <typename Duration>
class Chronometer
{
  private:
    using Clock = std::chrono::steady_clock;

    Duration::rep     elapsed_time_   = 0;
    Clock::time_point last_read_time_ = Clock::now();
    float             speed_          = 1;

  public:
    void
    reset()
    {
        elapsed_time_ = 0;
    }

    float
    speed() const
    {
        return speed_;
    }

    void
    speed(float s)
    {
        speed_ = s;
    }

    struct Reading
    {
        Duration::rep elapsed; // Time since the chronometer was started
        Duration::rep delta;   // Time since the last reading
    };

    Reading
    read()
    {
        auto delta_time = Duration(Clock::now() - last_read_time_).count() * speed_;

        elapsed_time_ += delta_time;

        last_read_time_ = Clock::now();

        return Reading{ .elapsed = elapsed_time_, .delta = delta_time };
    }
};

} // namespace ome
