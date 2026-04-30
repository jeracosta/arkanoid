#pragma once

#include <SDL2/SDL.h>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <ranges>
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
    struct ActionSlot_
    {
        EventBus<KeyboardInput> bus;
        std::size_t             pressed_count = 0;
    };

    std::flat_multimap<KeyboardInput, Action> input_actions_;
    std::vector<ActionSlot_>                  action_slots_;

    static std::size_t
    index_of_(Action action) noexcept
    {
        // NOTE: Here we assume that Action values are 0-indexed and contiguous.
        return static_cast<std::size_t>(action);
    }

    auto
    actions_for_(KeyboardInput input) const
    {
        auto [begin, end] = input_actions_.equal_range(input);
        return std::ranges::subrange(begin, end) | std::views::values;
    }

    ActionSlot_ &
    slot_of_(Action action)
    {
        auto index = index_of_(action);

        if (index >= action_slots_.size())
        {
            action_slots_.resize(index + 1);
        }

        return action_slots_[index];
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
    [[nodiscard]] decltype(auto)
    bind(Action action, Args &&...args)
    {
        return slot_of_(action).bus.bind(std::forward<Args>(args)...);
    }

    [[nodiscard]] bool
    is_pressed(Action action) const noexcept
    {
        auto index = index_of_(action);
        return index < action_slots_.size() && action_slots_[index].pressed_count != 0;
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

        const bool is_down = input.key_input != KeyInput::Release;

        for (auto action : actions_for_(input))
        {
            auto &slot = slot_of_(action);

            if (is_down)
            {
                ++slot.pressed_count;
            }
            else if (slot.pressed_count != 0)
            {
                --slot.pressed_count;
            }

            slot.bus.emit(input);
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
