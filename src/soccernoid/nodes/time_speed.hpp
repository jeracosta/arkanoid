#include "oh-my-engine/game.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/actions.hpp"

namespace soccernoid {

class TimeSpeedNode : public ome::Node
{
  private:
    float scaling_factor_ = 1.1f;

    ome::Interpolation<float>::Process speed_interpolation_
        = { 0.0f, 1.0f, ome::EasingCurve::smoothstep(1), 2.5 };

    void
    speed_by_(float factor_)
    {
        auto speed = speed_interpolation_.to() * factor_;

        speed_interpolation_.to(speed);

        log(std::format("Time speed: {}", speed));
    }

    void
    on_toggle_pause_()
    {
        speed_interpolation_.reverse();

        log(speed_interpolation_.is_reversed() ? "Paused" : "Resumed");
    }

  public:
    void
    on_mount_() override
    {
        hold(game()->input.bind(Action::TimeSpeedUp, [&] { speed_by_(scaling_factor_); }));
        hold(game()->input.bind(Action::TimeSpeedDown, [&] { speed_by_(1.0f / scaling_factor_); }));

        hold(game()->input.bind(Action::TogglePause, [&] { on_toggle_pause_(); }));
    }

    void
    on_tick_() override
    {
        speed_interpolation_.update(game()->time.unscaled.delta());
        game()->time.scale(speed_interpolation_.value());
    }
};

} // namespace soccernoid
