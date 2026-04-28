#pragma once

#include <boost/callable_traits/args.hpp>
#include <boost/mp11.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ome {

template <class... TEvents>
class EventDispatcher; // forward declaration

// Represents a subscription to an event.
// The registered callback will be invoked for the event as long as the connection is alive.
struct EventConnection
{
  private:
    // Parameter type-erased to support different event types while
    // allowing heterogeneous storage of connections.
    std::function<void(void *event)> callback_;

    // If set, connection will automatically disconnect when the owner expires.
    std::optional<std::weak_ptr<void>> owner_;

    bool
    owner_expired_() const
    {
        return owner_.has_value() && owner_->expired();
    }

    template <class... TEvents>
    friend class EventDispatcher;

  public:
    template <class T>
    void
    tie_to(const std::shared_ptr<T> &owner)
    {
        owner_ = owner;
    }
};

template <class... TEvents>
class EventDispatcher
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

  public:
    // Registers a callback for an event and returns ownership of a handle to the connection.
    // while that handle remains alive, the callback is invoked whenever the event is emitted.
    // The callback must be invocable with a single argument of a const ref to the event type.
    template <class TCallback>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallback &&callback)
    {
        using CallbackArgs = boost::callable_traits::args_t<TCallback>;
        using TEvent       = std::remove_cvref_t<std::tuple_element_t<0, CallbackArgs>>;
        static_assert(std::is_invocable_v<TCallback, const TEvent &>);

        static_assert(supported_<TEvent>,
                      "Tried binding to an event that is not supported by the dispatcher.");

        static_assert(std::is_invocable_v<TCallback, const TEvent &>,
                      "Tried binding a callback that cannot be invoked with the event type.");

        auto adapted_callback = [callback = std::forward<TCallback>(callback)](void *event)
        { callback(*static_cast<const TEvent *>(event)); };

        auto connection       = std::make_shared<EventConnection>();
        connection->callback_ = std::move(adapted_callback);

        connections_<TEvent>().push_back(connection);

        return connection;
    }

    template <class T, class Event>
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
            auto connection = it->lock();

            if (!connection || connection->owner_expired_())
            {
                it = connections.erase(it);
                continue;
            }

            std::invoke(connection->callback_, const_cast<TEvent *>(&event));
            ++it;
        }
    }
};

} // namespace ome
