#pragma once

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/node.hpp"

namespace ome {

// Adapts a node to produce events of the specified types, allowing observers to subscribe to them.
template <typename Base, typename... Events>
    requires std::derived_from<Base, Node>
class Eventful : public Base
{
  private:
    EventDispatcher<Events...> dispatcher_;

  protected:
    template <class TEvent>
    void
    emit_(const TEvent &event)
    {
        dispatcher_.template emit<TEvent>(event);
    }

  public:
    using Base::Base; // inherit constructors

    template <class TEvent>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(auto &&callback)
    {
        return dispatcher_.template bind<TEvent>(std::forward<decltype(callback)>(callback));
    }
};
} // namespace ome
