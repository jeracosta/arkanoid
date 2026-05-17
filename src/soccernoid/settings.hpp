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

namespace soccernoid::settings::time {

struct Paused
{
    bool value = false;

    constexpr Paused(bool value = false)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const Paused &) const = default;
};

struct Speed
{
    float value = 1.0f;

    constexpr Speed(float value = 1.0f)
        : value(value)
    {
    }

    constexpr
    operator float() const
    {
        return value;
    }

    constexpr bool
    operator==(const Speed &) const = default;
};

} // namespace soccernoid::settings::time

namespace soccernoid {

enum class CameraView
{
    FirstPerson,
    ThirdPerson,
    Count_,
};

inline CameraView
succesor(CameraView view)
{
    return static_cast<CameraView>((static_cast<int>(view) + 1)
                                   % static_cast<int>(CameraView::Count_));
}

} // namespace soccernoid

namespace soccernoid::settings::camera {

struct View
{
    CameraView value = CameraView::ThirdPerson;

    constexpr View(CameraView value = CameraView::ThirdPerson)
        : value(value)
    {
    }

    constexpr
    operator CameraView() const
    {
        return value;
    }

    constexpr bool
    operator==(const View &) const = default;
};

struct MouseSensitivity
{
    float value = 0.01f;

    constexpr MouseSensitivity(float value = 0.01f)
        : value(value)
    {
    }

    constexpr
    operator float() const
    {
        return value;
    }

    constexpr bool
    operator==(const MouseSensitivity &) const = default;
};

} // namespace soccernoid::settings::camera

namespace soccernoid {

class Settings : private ome::EventBus<settings::window::Fullscreen,
                                       settings::time::Paused,
                                       settings::time::Speed,
                                       settings::camera::View,
                                       settings::camera::MouseSensitivity>
{
  private:
    using EventBus_ = ome::EventBus<settings::window::Fullscreen,
                                    settings::time::Paused,
                                    settings::time::Speed,
                                    settings::camera::View,
                                    settings::camera::MouseSensitivity>;

    std::tuple<settings::window::Fullscreen,
               settings::time::Paused,
               settings::time::Speed,
               settings::camera::View,
               settings::camera::MouseSensitivity>
        values_;

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
