#pragma once

#include <boost/callable_traits/args.hpp>
#include <boost/mp11.hpp>
#include <functional>
#include <memory>
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

// Class that holds ownership of event connections, keeping them alive until the holder is destroyed
// or explicitly drops them.
class EventConnectionHolder
{
  private:
    std::vector<std::shared_ptr<EventConnection>> connections_;

  public:
    void
    hold(std::shared_ptr<EventConnection> connection)
    {
        connections_.push_back(std::move(connection));
    }

  protected:
    void
    drop_connections_()
    {
        connections_.clear();
    }
};

template <class... TEvents>
class EventBus
{
  private:
    using Events_ = boost::mp11::mp_list<TEvents...>;

    template <class TEvent>
    using ConnectionVector_ = std::vector<std::weak_ptr<EventConnection>>;

    using ConnectionMatrix_
        = boost::mp11::mp_rename<boost::mp11::mp_transform<ConnectionVector_, Events_>, std::tuple>;

    ConnectionMatrix_ callback_matrix_;

    template <class TEvent>
    ConnectionVector_<TEvent> &
    connections_()
    {
        constexpr auto index = boost::mp11::mp_find<Events_, TEvent>::value;
        return std::get<index>(callback_matrix_);
    }

    template <class TEvent>
    static constexpr bool supported_ = boost::mp11::mp_contains<Events_, TEvent>::value;

    // metaprogramming helper
    template <class TCallback>
    using arg0_t_ = std::tuple_element_t<0, boost::callable_traits::args_t<TCallback>>;

  public:
    // Registers a callback for an event and returns ownership of a handle to the connection.
    // While that handle remains alive, the callback is invoked whenever the event is emitted.
    template <class TCallback, class TEvent = std::remove_cvref_t<arg0_t_<TCallback>>>
        requires supported_<TEvent> && std::is_invocable_v<TCallback, const TEvent &>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallback &&callback)
    {
        auto adapted_callback = [callback = std::forward<TCallback>(callback)](void *event)
        { callback(*static_cast<const TEvent *>(event)); };

        auto connection       = std::make_shared<EventConnection>();
        connection->callback_ = std::move(adapted_callback);

        connections_<TEvent>().push_back(connection);

        return connection;
    }

    // Adapts bind to support callbacks that take no parameters, by ignoring the event parameter.
    template <class TEvent, class TCallback>
        requires supported_<TEvent> && std::is_invocable_v<TCallback>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallback &&callback)
    {
        auto adapted_callback
            = [callback = std::forward<TCallback>(callback)](const TEvent &) { callback(); };

        return bind(adapted_callback);
    }

    // Adapts bind to support member function pointers as callbacks, with an implicit instance
    // parameter.
    template <class T, class Event>
        requires supported_<Event>
                 && std::is_invocable_v<void (T::*)(const Event &), T *, const Event &>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(void (T::*member)(const Event &), T *instance)
    {
        auto callback = [instance, member](const Event &event) { (instance->*member)(event); };

        return bind(callback);
    }

    // Calls all registered callbacks with live connections to the event.
    template <class TEvent>
        requires supported_<TEvent>
    void
    emit(const TEvent &event)
    {
        auto &connections = connections_<TEvent>();

        for (auto it = connections.begin(); it != connections.end();)
        {
            if (auto connection = it->lock())
            {
                std::invoke(connection->callback_, const_cast<TEvent *>(&event));
                ++it;
            }
            else
            {
                it = connections.erase(it);
            }
        }
    }
};

} // namespace ome
