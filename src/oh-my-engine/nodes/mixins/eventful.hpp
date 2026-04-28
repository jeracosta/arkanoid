#pragma once

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/member_forwarding_macro.hpp"
#include "oh-my-engine/node.hpp"

namespace ome {

// Adapts a node to produce events of the specified types, allowing observers to subscribe to them.
template <typename Base, typename... Events>
    requires std::derived_from<Base, Node>
class Eventful : public Base, protected EventBus<Events...>
{
  protected:
    using EventBus<Events...>::emit_;

  public:
    using EventBus<Events...>::bind;
};

} // namespace ome
