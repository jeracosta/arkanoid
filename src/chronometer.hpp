#pragma once
#include <chrono>

template <typename Duration>
class Chronometer
{
  private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point start_time_ = Clock::now();
    Clock::time_point last_time_  = start_time_;

  public:
    void
    reset()
    {
        start_time_ = Clock::now();
        last_time_  = start_time_;
    }

    struct Reading
    {
        Duration::rep elapsed; // Time since the chronometer was started
        Duration::rep delta;   // Time since the last reading
    };

    Reading
    read()
    {
        auto now = Clock::now();

        auto countSince = [&now](Clock::time_point time)
        {
            return std::chrono::duration_cast<Duration>(now - time).count();
        };

        auto reading = Reading{
            .elapsed = countSince(start_time_),
            .delta   = countSince(last_time_),
        };

        last_time_ = now;

        return reading;
    }
};
