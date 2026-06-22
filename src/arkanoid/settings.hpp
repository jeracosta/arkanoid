#pragma once

#include <utility>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/events.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace arkanoid::settings::window {

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

} // namespace arkanoid::settings::window

namespace arkanoid::settings::time {

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

} // namespace arkanoid::settings::time

namespace arkanoid {

enum class TextFont
{
    Arial,
    ComicSans,
    DejaVuSans,
    Count_,
};

enum class CameraView
{
    ThirdPerson,
    ChaseCamera,
    Freecam,
    Count_,
};

inline CameraView
succesor(CameraView view)
{
    return static_cast<CameraView>((static_cast<int>(view) + 1)
                                   % static_cast<int>(CameraView::Count_));
}

} // namespace arkanoid

namespace arkanoid::settings::camera {

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

struct MovementSpeed
{
    float value = 5.0f;

    constexpr MovementSpeed(float value = 5.0f)
        : value(value)
    {
    }

    constexpr
    operator float() const
    {
        return value;
    }

    constexpr bool
    operator==(const MovementSpeed &) const = default;
};

} // namespace arkanoid::settings::camera

namespace arkanoid::settings::render {

struct ShowFrameRate
{
    bool value = true;

    constexpr ShowFrameRate(bool value = true)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const ShowFrameRate &) const = default;
};

struct ShowWireframes
{
    bool value = false;

    constexpr ShowWireframes(bool value = false)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const ShowWireframes &) const = default;
};

struct ShowTextures
{
    bool value = true;

    constexpr ShowTextures(bool value = true)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const ShowTextures &) const = default;
};

// true: interpolado (GL_SMOOTH) · false: facetado (GL_FLAT)
struct SmoothShading
{
    bool value = true;

    constexpr SmoothShading(bool value = true)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const SmoothShading &) const = default;
};

struct ShowHitboxes
{
    bool value = false;

    constexpr ShowHitboxes(bool value = false)
        : value(value)
    {
    }

    constexpr
    operator bool() const
    {
        return value;
    }

    constexpr bool
    operator==(const ShowHitboxes &) const = default;
};

// Index into the dynamically loaded TexturePalette::skybox array.
struct Skybox
{
    int value = 0;

    constexpr Skybox(int value = 0)
        : value(value)
    {
    }

    constexpr
    operator int() const
    {
        return value;
    }

    constexpr bool
    operator==(const Skybox &) const = default;
};

} // namespace arkanoid::settings::render

namespace arkanoid::settings::ui {

struct TextFont
{
    arkanoid::TextFont value = arkanoid::TextFont::Arial;

    constexpr TextFont(arkanoid::TextFont value = arkanoid::TextFont::Arial)
        : value(value)
    {
    }

    constexpr
    operator arkanoid::TextFont() const
    {
        return value;
    }

    constexpr bool
    operator==(const TextFont &) const = default;
};

} // namespace arkanoid::settings::ui

namespace arkanoid::settings {

struct GlobalLight
{
    ome::Vec3f direction = ome::down;

    ome::Color ambient  = ome::Color::rgb(0.04f, 0.04f, 0.04f);
    ome::Color diffuse  = ome::Color::rgb(0.10f, 0.10f, 0.10f);
    ome::Color specular = ome::Color::rgb(0.02f, 0.02f, 0.02f);

    bool
    operator==(const GlobalLight &) const = default;
};

} // namespace arkanoid::settings

namespace arkanoid {

using SettingsEventBus = ome::EventBus<settings::window::Fullscreen,
                                       settings::time::Paused,
                                       settings::time::Speed,
                                       settings::camera::View,
                                       settings::camera::MouseSensitivity,
                                       settings::camera::MovementSpeed,
                                       settings::render::ShowFrameRate,
                                       settings::render::ShowWireframes,
                                       settings::ui::TextFont,
                                       settings::render::ShowTextures,
                                       settings::render::SmoothShading,
                                       settings::render::ShowHitboxes,
                                       settings::render::Skybox,
                                       settings::GlobalLight>;

class Settings : private SettingsEventBus
{
  private:
    SettingsEventBus::EventTuple values_;

  public:
    using SettingsEventBus::bind;

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
        emit(current);
    }
};

} // namespace arkanoid
