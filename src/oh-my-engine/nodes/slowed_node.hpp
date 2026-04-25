#pragma once

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/time.hpp"

namespace ome {

// Adapts a node to only tick at a specified period, effectively slowing it down by skipping ticks
// in between. It does not slow down instance-level hooks.
template <typename Base, float Period, typename Duration = ome::Time::Unit>
    requires std::derived_from<Base, Node>
class Slowed : public Base
{
  private:
    float cooldown_ = 0;

    static constexpr auto period
        = std::chrono::duration_cast<ome::Time::Unit>(Duration{ Period }).count();

  public:
    template <typename... Args>
    Slowed(Args &&...args)
        : Base(std::forward<Args>(args)...)
    {
    }

    void
    tick_() override
    {
        cooldown_ -= Node::game()->time.delta();

        if (cooldown_ <= 0)
        {
            Base::tick_();
            cooldown_ = Period;
        }
    }
};
} // namespace ome
