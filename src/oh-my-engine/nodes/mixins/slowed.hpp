#pragma once

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/time.hpp"

namespace ome {

// Adapts a node to only tick at a specified period, effectively slowing it down by skipping ticks
// in between. It does not slow down instance-level hooks.
template <std::derived_from<Node> TBase, float TPeriod, typename TDuration = ome::Time::Unit>
class Slowed : public TBase
{
  private:
    float cooldown_ = 0;

    static constexpr auto period
        = std::chrono::duration_cast<ome::Time::Unit>(TDuration{ TPeriod }).count();

  public:
    template <typename... Args>
    Slowed(Args &&...args)
        : TBase(std::forward<Args>(args)...)
    {
    }

    void
    on_tick_() override
    {
        cooldown_ -= Node::game()->time.unscaled.delta();

        if (cooldown_ <= 0)
        {
            TBase::on_tick_();
            cooldown_ = TPeriod;
        }
    }
};
} // namespace ome
