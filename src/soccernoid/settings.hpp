#pragma once

#include <tuple>
#include <utility>

#include "oh-my-engine/events.hpp"

namespace soccernoid::settings::window {

struct Fullscreen
{
    bool value = false;

    constexpr Fullscreen(bool value = false)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const Fullscreen &) const = default;
};

} // namespace soccernoid::settings::window

namespace soccernoid {

class Settings : private ome::EventBus<settings::window::Fullscreen>
{
  private:
    using EventBus_ = ome::EventBus<settings::window::Fullscreen>;

    std::tuple<settings::window::Fullscreen> values_;

  public:
    using EventBus_::bind;

    template <class T>
    const T &
    get() const
    {
        return std::get<T>(values_);
    }

    template <class T>
    void
    set(T value)
    {
        auto &current = std::get<T>(values_);
        if (current == value)
        {
            return;
        }

        current = std::move(value);
        EventBus_::emit(current);
    }
};

} // namespace soccernoid
