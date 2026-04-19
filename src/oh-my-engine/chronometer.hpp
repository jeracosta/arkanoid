#pragma once
#include <chrono>

namespace ome {

template <typename TDuration, class TClock = std::chrono::steady_clock>
class Chronometer
{
  private:
    TDuration::rep     elapsed_time_   = 0;
    TClock::time_point last_read_time_ = TClock::now();
    float              scale_          = 1;

  public:
    using Clock = TClock;

    void
    reset()
    {
        elapsed_time_ = 0;
    }

    float
    scale() const
    {
        return scale_;
    }

    void
    scale(float s)
    {
        scale_ = s;
    }

    struct Reading
    {
        TDuration::rep elapsed; // Time since the chronometer was started
        TDuration::rep delta;   // Time since the last reading
    };

    Reading
    read()
    {
        auto delta_time = TDuration(TClock::now() - last_read_time_).count() * scale_;

        elapsed_time_ += delta_time;

        last_read_time_ = TClock::now();

        return Reading{ .elapsed = elapsed_time_, .delta = delta_time };
    }
};

} // namespace ome
