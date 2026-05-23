#pragma once

#include "oh-my-engine/node.hpp"

namespace ome {

// Adapts a node to provide despawning functionality: it will die when a certain condition is met.
template <std::derived_from<ome::Node> TBase, typename TShouldDespawn>
class Despawning : public TBase
{
  public:
    void
    on_tick_() override
    {
        [[unlikely]]
        if (TShouldDespawn{}(*this))
        {
            this->request_unmount();
        }

        TBase::on_tick_();
    }
};

} // namespace ome
