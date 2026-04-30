#include "oh-my-engine/node.hpp"
#include "soccernoid/actions.hpp"

namespace soccernoid {

class TimeSpeedNode : public ome::Node
{
  private:
    float scaling_factor_ = 1.1f;

    void
    speed_by_(float factor_)
    {
        auto speed = game()->time.scale() * factor_;

        game()->time.scale(speed);

        log(std::format("Time speed: {}", speed));
    }

  public:
    void
    on_mount_() override
    {
        hold(game()->input.bind(Action::TimeSpeedUp, [&] { speed_by_(scaling_factor_); }));
        hold(game()->input.bind(Action::TimeSpeedDown, [&] { speed_by_(1.0f / scaling_factor_); }));
    }
};

} // namespace soccernoid
