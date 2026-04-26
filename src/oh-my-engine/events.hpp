#pragma once

#include <boost/mp11.hpp>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ome {

template <class TTypes>
struct events_specification
{
    using Tuple = std::tuple<>;

    template <class TEvent>
    static constexpr bool is_event = false;
};

template <class TTypes>
    requires requires { std::tuple_size<TTypes>::value; }
struct events_specification<TTypes>
{
    using Tuple = TTypes;

    template <class TEvent>
    static constexpr bool is_event = boost::mp11::mp_contains<Tuple, TEvent>::value;

    template <class TEvent>
        requires is_event<TEvent>
    static constexpr std::size_t code_of = boost::mp11::mp_find<Tuple, TEvent>::value;
};

#define DEFINE_EVENTS(...) using Events = ::ome::events_specification<std::tuple<__VA_ARGS__>>

template <class TEmitter, class TEvent, class TCallable>
static constexpr bool handles_event = TEmitter::Events::template is_event<TEvent>
                                      && std::is_invocable_v<TCallable, TEmitter &, const TEvent &>;

template <class TEmitter, class TCallable>
static constexpr bool handles_any_event = std::apply([](auto... events) constexpr
{ return (std::is_invocable_v<TCallable, TEmitter &, decltype(events)> || ...); },
                                                     typename TEmitter::Events::Tuple{});

// Component of a class (TEmitter) that can dispatch events to registered callbacks.
template <class TEmitter>
    requires requires { typename TEmitter::Events; }
class EventDispatcher
{
  private:
    TEmitter &emitter_;

    enum class CallbackResponse_
    {
        Nil = 0, // default value representing no response
        Unbind,  // callback requested to stop being called
    };

    template <class TEvent>
    using Callback_ = std::function<CallbackResponse_(TEmitter &, const TEvent &)>;

    template <class TEvent>
    using CallbackVector_ = std::vector<Callback_<TEvent>>;

    using CallbackMatrix_
        = boost::mp11::mp_transform<CallbackVector_, typename TEmitter::Events::Tuple>;

    CallbackMatrix_ callback_matrix_;

    template <class TEvent>
    auto &
    callbacks_for_()
    {
        return std::get<TEvent>(callback_matrix_);
    }

  public:
    template <class TEvent, class TObserver, class TCallable>
    void
    bind(TCallable TObserver::*method, std::weak_ptr<TObserver> observer)
    {
        auto callback
            = [method, observer](TEmitter &emitter, const TEvent &event) -> CallbackResponse_
        {
            if (auto obs = observer.lock())
            {
                (obs.get()->*method)(emitter, event);
                return CallbackResponse_::Nil;
            }
            else
            {
                return CallbackResponse_::Unbind;
            }
        };

        static_assert(handles_event<TEmitter, TEvent, decltype(callback)>);

        callbacks_for_<TEvent>().emplace_back(std::move(callback));
    }

    template <class TEvent>
        requires TEmitter::Events::template
    is_event<TEvent> void
    dispatch(const TEvent &event)
    {
        auto &callbacks = callbacks_for_<TEvent>();

        for (auto it = callbacks.begin(); it != callbacks.end();)
        {
            if ((*it)(emitter_, event) == CallbackResponse_::Unbind)
            {
                it = callbacks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};

} // namespace ome
