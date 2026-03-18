#include <SDL2/SDL.h>
#include <actions.hpp>
#include <array>
#include <bitset>
#include <cstdint>
#include <ranges>
#include <variant>

struct Key
{
    using Code = SDL_Scancode;

    static constexpr auto CODE_COUNT = SDL_NUM_SCANCODES; // number of possible key codes.

    Code code;
};

struct MouseButton
{
    using Code = uint8_t; // use `SDL_BUTTON_*` macros, e.g. `SDL_BUTTON_LEFT`

    static constexpr auto CODE_COUNT = sizeof(Code); // number of possible mouse button codes.

    Code code;
};

using DiscreteInput = std::variant<Key, MouseButton>;

// Data about a specific discrete input.
// It's a proxy object that should dissapear in optimized compilation.
template <typename Input>
class DiscreteInputState
{
  private:
    ActionMask                               &action_mask;
    std::bitset<Input::CODE_COUNT>::reference pressed;

  public:
    DiscreteInputState(auto &&action_mask, auto &&pressed)
        : action_mask(action_mask),
          pressed(pressed)
    {
    }

    void
    bind(Action action)
    {
        action_mask |= mask(action);
    }

    void
    unbind(Action action)
    {
        action_mask &= ~mask(action);
    }

    bool
    is_pressed()
    {
        return pressed;
    }

    friend class InputMap;
};

class InputMap
{
  private:
    template <typename Input>
    class DiscreteInputStateStorage
    {
      private:
        // Data layout designed for cache-friendliness.
        std::array<ActionMask, Input::CODE_COUNT> actions_{};
        std::bitset<Input::CODE_COUNT>            pressed_{};

      public:
        DiscreteInputState<Input>
        operator[](Input::Code code)
        {
            return { actions_[code], pressed_[code] };
        }

        ActionMask
        get_triggered_actions_mask() const
        {
            ActionMask mask{};

            for (auto i : std::ranges::views::iota(size_t{ 0 }, actions_.size()))
            {
                if (pressed_[i])
                {
                    mask |= actions_[i];
                }
            }

            return mask;
        }
    };

    DiscreteInputStateStorage<Key>         keys;
    DiscreteInputStateStorage<MouseButton> mouse_buttons;

  public:
    using Binding = std::pair<DiscreteInput, std::initializer_list<Action>>;

    InputMap(std::initializer_list<Binding> init)
    {
        for (auto const &b : init)
        {
            std::visit(
                [this, &b](auto const &input)
                {
                    using T = std::decay_t<decltype(input)>;
                    for (Action a : b.second)
                    {
                        if constexpr (std::is_same_v<T, Key>)
                            keys[input.code].bind(a);
                        else
                            mouse_buttons[input.code].bind(a);
                    }
                },
                b.first);
        }
    }

    auto
    operator[](Key key)
    {
        return keys[key.code];
    }

    auto
    operator[](MouseButton button)
    {
        return mouse_buttons[button.code];
    }

    void
    handle(const SDL_Event &event)
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            keys[event.key.keysym.scancode].pressed = true;
            break;
        case SDL_KEYUP:
            keys[event.key.keysym.scancode].pressed = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouse_buttons[event.button.button].pressed = true;
            break;
        case SDL_MOUSEBUTTONUP:
            mouse_buttons[event.button.button].pressed = false;
            break;
        default:
            break; // ignore other events
        }
    }

    ActionMask
    get_triggered_actions_mask() const
    {
        return keys.get_triggered_actions_mask() | mouse_buttons.get_triggered_actions_mask();
    }
};
