#pragma once

#include <boost/callable_traits/args.hpp>
#include <boost/mp11.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ome {

template <class... TEvents>
class EventBus; // forward declaration

// Represents a subscription to an event.
// The registered callback will be invoked for the event as long as the connection is alive.
// Handle-like class, tighly coupled to the event bus instance that produced it.
struct EventConnection : public std::enable_shared_from_this<EventConnection>
{
  private:
    // The callable to be invoked when the event is emitted.
    // Its parameter is type-erased to support different event types while
    // allowing homogeneous storage of connections.
    std::function<void(void *event)> callback_;

    template <class... TEvents>
    friend class EventBus;
};

template <class... TEvents>
class EventBus
{
  private:
    using Events_ = boost::mp11::mp_list<TEvents...>;

    struct ConnectionSlot_
    {
        std::weak_ptr<EventConnection>     connection;
        std::shared_ptr<EventConnection>   ownership;
        std::optional<std::weak_ptr<void>> lifetime_guard;
    };

    template <class TEvent>
    using ConnectionVector_ = std::vector<std::weak_ptr<ConnectionSlot_>>;

    using ConnectionMatrix_
        = boost::mp11::mp_rename<boost::mp11::mp_transform<ConnectionVector_, Events_>, std::tuple>;

    ConnectionMatrix_ callback_matrix_;

    template <class TEvent>
    ConnectionVector_<TEvent> &
    connection_slots_()
    {
        constexpr auto index = boost::mp11::mp_find<Events_, TEvent>::value;
        return std::get<index>(callback_matrix_);
    }

    template <class TEvent>
    static constexpr bool supported_ = boost::mp11::mp_contains<Events_, TEvent>::value;

    // metaprogramming helper
    template <class TCallback>
    using arg0_t_ = std::tuple_element_t<0, boost::callable_traits::args_t<TCallback>>;

  protected:
    // Calls all registered callbacks with live connections to the event.
    template <class TEvent>
        requires supported_<TEvent>
    void
    emit_(const TEvent &event)
    {
        auto &slots = connection_slots_<TEvent>();

        for (auto it = slots.begin(); it != slots.end();)
        {
            auto &connection = it->connection.lock();

            {
                auto &guard = it->lifetime_guard;

                if (guard.has_value() && guard->expired())
                {
                    guard.reset();
                    connection.ownership.reset();
                }
            }

            if (connection)
            {
                std::invoke(connection->callback_, const_cast<TEvent *>(&event));
                ++it;
            }
            else
            {
                it = slots.erase(it);
            }
        }
    }

  public:
    // Registers a callback for an event and returns ownership of a handle to the connection.
    // While that handle remains alive, the callback is invoked whenever the event is emitted.
    // If `lifetime_guard` is set, the connection is kept alive while its guard is alive.
    template <class TCallback, class TEvent = std::remove_cvref_t<arg0_t_<TCallback>>>
        requires supported_<TEvent> && std::is_invocable_v<TCallback, const TEvent &>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallback &&callback, std::weak_ptr<void> lifetime_guard = {})
    {
        auto adapted_callback = [callback = std::forward<TCallback>(callback)](void *event)
        { std::invoke(callback, *static_cast<const TEvent *>(event)); };

        auto connection       = std::make_shared<EventConnection>();
        connection->callback_ = std::move(adapted_callback);

        auto slot = ConnectionSlot_{ connection };

        if (!lifetime_guard.expired())
        {
            slot.lifetime_guard = std::move(lifetime_guard);
            slot.ownership      = connection;
        }

        connection_slots_<TEvent>().push_back(slot);

        return connection;
    }

    // Adapts `bind` method to support callbacks with no parameters, by ignoring the event value.
    template <class TEvent, class TCallback>
        requires std::is_invocable_v<TCallback>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallback &&callback, std::weak_ptr<void> lifetime_guard = {})
    {
        auto adapted_callback
            = [callback = std::forward<TCallback>(callback)](const TEvent &) { callback(); };

        return bind(adapted_callback, lifetime_guard);
    }

    // Adapts `bind` method to support member function pointers as callbacks, with an implicit
    // instance parameter that doubles as a lifetime guard.
    template <class T, class Event>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(void (T::*member)(const Event &), std::weak_ptr<T> instance)
    {
        auto callback = [instance, member](const Event &event) { (instance->*member)(event); };

        return bind(callback, instance);
    }
};

} // namespace ome
