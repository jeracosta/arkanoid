#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

class TimeSpeedNode : public SoccernoidNode<>
{
  private:
    using Paused = settings::time::Paused;

    float scaling_factor_ = 1.1f;

    std::shared_ptr<ome::Interpolation<float>> speed_curve_
        = std::make_shared<ome::Interpolation<float>>(0.0f, 1.0f, ome::EasingCurve::smoothstep(1));

    ome::CurveProcess<float> speed_interpolation_{ speed_curve_, 2.5f };

    void
    speed_by_(float factor_)
    {
        auto speed = speed_curve_->to() * factor_;

        speed_curve_->to(speed);

        log(std::format("Time speed: {}", speed));
    }

    void
    set_paused_(Paused paused)
    {
        if (speed_interpolation_.is_reversed() == static_cast<bool>(paused))
        {
            return;
        }

        speed_interpolation_.reverse();

        log(paused ? "Paused" : "Resumed");
    }

  public:
    void
    on_mount_() override
    {
        hold(game()->input.bind(Action::TimeSpeedUp, [&] { speed_by_(scaling_factor_); }));
        hold(game()->input.bind(Action::TimeSpeedDown, [&] { speed_by_(1.0f / scaling_factor_); }));

        hold(game()->input.bind(Action::TogglePause, [&] {
            game()->settings.set<Paused>(!game()->settings.get<Paused>());
        }));

        hold(game()->settings.bind([this](const Paused &paused) { set_paused_(paused); }));
    }

    void
    on_tick_() override
    {
        speed_interpolation_.update(game()->time.unscaled.delta());
        game()->time.scale(speed_interpolation_.value());
    }
};

} // namespace soccernoid
