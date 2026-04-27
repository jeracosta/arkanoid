#pragma once

#include <boost/mp11.hpp>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

namespace ome {

template <class... TEvents>
class EventDispatcher; // forward declaration

struct EventConnection
{
  private:
    // Parameter type-erased to support different event types while
    // allowing heterogeneous storage of connections.
    std::function<void(void *event)> callback_;

    template <class... TEvents>
    friend class EventDispatcher;
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
    // Registers a callback for an event and returns a handle to the connection.
    // The callback is invoked whenever the event is emitted, receiving the event as an argument.
    // The subscription remains active while the returned connection handle is alive.
    template <class TEvent, class TCallable>
    [[nodiscard]] std::shared_ptr<EventConnection>
    bind(TCallable &&callback)
    {
        static_assert(supported_<TEvent>,
                      "Tried binding to an event that is not supported by the dispatcher.");

        static_assert(std::is_invocable_v<TCallable, const TEvent &>,
                      "Tried binding a callback that cannot be invoked with the event type.");

        auto adapted_callback = [callback = std::forward<TCallable>(callback)](void *event)
        { callback(*static_cast<const TEvent *>(event)); };

        auto connection       = std::make_shared<EventConnection>();
        connection->callback_ = std::move(adapted_callback);

        connections_<TEvent>().push_back(connection);

        return connection;
    }

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
