#pragma once

#include <SDL2/SDL.h>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome::input {

// Actions categorize keyboard inputs and represent game intent, such as "jump" or "move left".
// They are mapped to specific keyboard inputs.
// NOTE: The enum is left undefined to allow users to define their own actions as needed.
// WARN: For performance reasons, values should be 0-indexed and contiguous.
enum class Action;

enum class KeyInput : uint8_t
{
    Press,   // The key was pressed down.
    Repeat,  // The OS simulated a key press because it is being held down.
    Release, // The key was released (after being pressed).
};

struct KeyboardInput
{
    SDL_Keycode key_code;
    KeyInput    key_input;

    constexpr auto
    operator<=>(const KeyboardInput &) const
        = default;

    operator KeyInput() const
    {
        return key_input;
    }
};

class KeyboardInputMapper
{
  private:
    std::flat_multimap<KeyboardInput, Action> input_actions_;
    std::vector<EventBus<KeyboardInput>>      action_event_buses_;

    auto
    actions_for_(KeyboardInput input) const
    {
        auto [begin, end] = input_actions_.equal_range(input);
        return std::ranges::subrange(begin, end) | std::views::values;
    }

    EventBus<KeyboardInput> &
    event_bus_of_(Action action)
    {
        // Here we assume that actions are 0-indexed and contiguous.

        auto index = static_cast<size_t>(action);

        if (index >= action_event_buses_.size())
        {
            action_event_buses_.resize(index + 1);
        }

        return action_event_buses_[index];
    }

  public:
    void
    bind(SDL_Keycode key, KeyInput input, Action action)
    {
        input_actions_.emplace(KeyboardInput{ key, input }, action);
    }

    void
    bind(SDL_Keycode key, std::initializer_list<KeyInput> inputs, Action action)
    {
        for (auto input : inputs)
        {
            bind(key, input, action);
        }
    }

    void
    unbind(SDL_Keycode key, KeyInput input)
    {
        input_actions_.erase(KeyboardInput{ key, input });
    }

    template <typename... Args>
    decltype(auto)
    bind(Action action, Args &&...args)
    {
        auto &event_bus = event_bus_of_(action);
        return event_bus.bind(std::forward<Args>(args)...);
    }

    void
    handle(const SDL_Event &event)
    {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
        {
            return;
        }

        auto input = KeyboardInput{
            .key_code  = event.key.keysym.sym,
            .key_input = event.key.repeat
                             ? KeyInput::Repeat
                             : (event.type == SDL_KEYDOWN ? KeyInput::Press : KeyInput::Release),
        };

        for (auto &action : actions_for_(input))
        {
            event_bus_of_(action).emit(input);
        }
    }
};

struct MouseMotionInput
{
    Vec2f delta;
    Vec2u position;
};

class MouseMotionInputMapper : private EventBus<MouseMotionInput>
{

  public:
    using EventBus::bind;

    void
    handle(const SDL_Event &event)
    {
        if (event.type != SDL_MOUSEMOTION)
        {
            return;
        }

        auto input = MouseMotionInput{
            .delta    = { event.motion.xrel, event.motion.yrel },
            .position = { event.motion.x, event.motion.y },
        };

        EventBus::emit(input);
    }
};

struct InputMapper
{
    KeyboardInputMapper    keyboard;
    MouseMotionInputMapper mouse_motion;

    void
    handle(const SDL_Event &event)
    {
        keyboard.handle(event);
        mouse_motion.handle(event);
    }
};

} // namespace ome::input
