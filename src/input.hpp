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
    std::flat_map<InputActionTrigger, InputAction> bindings_;

  public:
    constexpr auto &
    operator[](const InputActionTrigger &trigger)
    {
        return bindings_[trigger];
    }

    constexpr auto &
    operator[](SDL_KeyCode key, KeyInput input)
    {
        return bindings_[InputActionTrigger{ key, input }];
    }

    constexpr void
    bind(SDL_KeyCode key, KeyInput input, InputAction &&action)
    {
        [[unlikely]] if (bindings_.contains({ key, input }))
        {
            assert(!"Tried to bind an already bound input action trigger. "
                    "(multiple actions per trigger are not supported); "
                    "unbind it first.");
            return;
        }
        (*this)[key, input] = std::forward<InputAction>(action);
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
            if (auto obj = object.lock())
            {
                (obj.get()->*method)();
            }
            else
            {
                unbind(key, input);
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

        bindings_.contains({ key, input }) ? bindings_.at({ key, input })() : void();
    };
};
