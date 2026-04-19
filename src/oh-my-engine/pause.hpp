#include "time.hpp"

namespace ome {

class Pause
{
  private:
    Time                                      &time_;
    Chronometer<Time::Unit>::Clock::time_point paused_timestamp_;

  public:
    Pause(Time &time)
        : time_(time)
    {
    }

    bool
    is_paused() const
    {
        return time_.frozen_();
    }

    auto
    paused_at() const
    {
        return paused_timestamp_;
    }

    void
    toggle()
    {
        if (is_paused())
        {
            time_.unfreeze_();
        }
        else
        {
            time_.freeze_();
            paused_timestamp_ = Chronometer<Time::Unit>::Clock::now();
        }
    }
};

} // namespace ome
