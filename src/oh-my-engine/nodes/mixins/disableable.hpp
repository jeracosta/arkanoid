#pragma once

#include "oh-my-engine/node.hpp"

namespace ome {

// Adapts a node to provide a notion of being enabled.
// A disabled node doesn't tick and is effectively paused, but instance-level hooks still run.
template <std::derived_from<Node> TBase>
class Disableable : public TBase
{
  private:
    bool enabled_ = true;

  public:
    void
    tick_() override
    {
        [[likely]]
        if (enabled_)
        {
            TBase::tick_();
        }
    }

    bool
    is_enabled() const
    {
        return enabled_;
    }

    void
    enable()
    {
        enabled_ = true;
    }

    void
    disable()
    {
        enabled_ = false;
    }

    void
    toggle_enabled()
    {
        enabled_ = !enabled_;
    }
};
} // namespace ome
