#include <SDL2/SDL.h>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

enum class KeyInput : uint8_t
{
    Press,
    Repeat,
    Release,
};

using InputAction = std::function<void()>;

struct InputActionTrigger
{
    SDL_Keycode key;
    KeyInput    input;

    constexpr auto
    operator<=>(const InputActionTrigger &other) const
        = default;
};

class KeyboardInputMapper
{
  private:
    std::flat_multimap<InputActionTrigger, InputAction> bindings_;

    auto
    actions_for_(InputActionTrigger trigger) const
    {
        auto [begin, end] = bindings_.equal_range(trigger);
        return std::ranges::subrange(begin, end) | std::views::values;
    }

  public:
    constexpr void
    bind(SDL_KeyCode key, KeyInput input, InputAction &&action)
    {
        bindings_.emplace(InputActionTrigger{ key, input }, std::forward<InputAction>(action));
    }

    constexpr void
    bind(SDL_KeyCode key, std::initializer_list<KeyInput> inputs, InputAction &&action)
    {
        for (KeyInput input : inputs)
        {
            bind(key, input, std::forward<InputAction>(action));
        }
    }

    constexpr void
    unbind(SDL_KeyCode key, KeyInput input)
    {
        bindings_.erase(InputActionTrigger{ key, input });
    }

    template <class T>
    constexpr void
    bind(SDL_KeyCode key, KeyInput input, void (T::*method)(), T *object)
    {
        bind(key, input, [object, method]() { (object->*method)(); });
    }

    template <typename T>
    constexpr void
    bind(SDL_KeyCode key, KeyInput input, void (T::*method)(), std::shared_ptr<T> object)
    {
        bind(key,
             input,
             [this, object = std::weak_ptr(object), method, key, input]()
        {
            [[likely]]
            if (auto obj = object.lock())
            {
                (obj.get()->*method)();
            }
            else
            {
                throw std::runtime_error("Input binding target expired");
            }
        });
    }

    template <typename Method, typename Object>
    constexpr void
    bind(SDL_KeyCode key, std::initializer_list<KeyInput> inputs, Method method, Object object)
    {
        for (KeyInput input : inputs)
        {
            bind(key, input, method, object);
        }
    }

    void
    handle(const SDL_Event &event)
    {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
        {
            return;
        }

        auto key = event.key.keysym.sym;

        auto input = event.key.repeat
                         ? KeyInput::Repeat
                         : (event.type == SDL_KEYDOWN ? KeyInput::Press : KeyInput::Release);

        for (const auto &action : actions_for_({ key, input }))
        {
            action();
        }
    };
};
