#pragma once

#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <ranges>
#include <utility>

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/sdl/event_handler.hpp"

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

class KeyboardInputMapper : public ome::sdl::EventHandler
{
  private:
    struct ActionSlot_
    {
        EventBus<KeyboardInput>     bus;
        std::flat_set<SDL_Scancode> scancodes;
    };

    std::flat_multimap<KeyboardInput, Action> input_actions_;
    mutable std::vector<ActionSlot_>          action_slots_;

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
    slot_of_(Action action) const
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
    bind(KeyboardInput input, Action action)
    {
        input_actions_.emplace(input, action);

        if (auto scancode = SDL_GetScancodeFromKey(input.key_code))
        {
            slot_of_(action).scancodes.insert(scancode);
        }
    }

    void
    bind(SDL_Keycode key, KeyInput input, Action action)
    {
        bind(KeyboardInput{ key, input }, action);
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
    unbind(KeyboardInput input)
    {
        auto actions = [&]
        {
            auto [begin, end] = input_actions_.equal_range(input);
            return std::ranges::subrange(begin, end) | std::views::values;
        }();

        for (auto &action : actions)
        {
            if (auto scancode = SDL_GetScancodeFromKey(input.key_code))
            {
                slot_of_(action).scancodes.erase(scancode);
            }
        }

        input_actions_.erase(input);
    }

    template <typename... Args>
    [[nodiscard]] decltype(auto)
    bind(Action action, Args &&...args)
    {
        return slot_of_(action).bus.bind(std::forward<Args>(args)...);
    }

    template <typename TCallback>
        requires std::invocable<TCallback> // no-arg callback
    [[nodiscard]] decltype(auto)
    bind(Action action, TCallback &&callback)
    {
        return slot_of_(action).bus.bind<KeyboardInput>(std::forward<TCallback>(callback));
    }

    [[nodiscard]] bool
    is_pressed(Action action) const noexcept
    {
        const auto *state = SDL_GetKeyboardState(nullptr);

        for (auto scancode : slot_of_(action).scancodes)
        {
            if (state[scancode])
            {
                return true;
            }
        }

        return false;
    }

    std::optional<SDL_Event>
    handle(const SDL_Event &event) override
    {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
            return event;

        auto input = KeyboardInput{
            .key_code  = event.key.keysym.sym,
            .key_input = event.key.repeat
                             ? KeyInput::Repeat
                             : (event.type == SDL_KEYDOWN ? KeyInput::Press : KeyInput::Release),
        };

        for (auto action : actions_for_(input))
            slot_of_(action).bus.emit(input);

        return std::nullopt;
    }
};
;

struct MouseMotionInput
{
    Vec2f delta;
    Vec2u position;
};

class MouseMotionInputMapper : private EventBus<MouseMotionInput>, public ome::sdl::EventHandler
{

  public:
    using EventBus::bind;

    std::optional<SDL_Event>
    handle(const SDL_Event &event)
    {
        if (event.type != SDL_MOUSEMOTION)
        {
            return event; // didn't consume the event
        }

        auto input = MouseMotionInput{
            .delta    = { event.motion.xrel, event.motion.yrel },
            .position = { event.motion.x, event.motion.y },
        };

        EventBus::emit(input);

        return std::nullopt; // consumed the event
    }
};

struct InputMapper : public KeyboardInputMapper, public MouseMotionInputMapper
{
    std::optional<SDL_Event>
    handle(const SDL_Event &event) override
    {
        if (!KeyboardInputMapper::handle(event).has_value())
        {
            return std::nullopt;
        }

        if (!MouseMotionInputMapper::handle(event).has_value())
        {
            return std::nullopt;
        }

        return event;
    }

    using KeyboardInputMapper::bind;
    using MouseMotionInputMapper::bind;
};

} // namespace ome::input
