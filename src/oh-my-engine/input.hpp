#include <SDL2/SDL.h>

#include <cassert>
#include <cstdint>
#include <flat_map>
#include <functional>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace ome::input {

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
    operator<=>(const InputActionTrigger &) const
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
    void
    bind(SDL_Keycode key, KeyInput input, InputAction action)
    {
        bindings_.emplace(InputActionTrigger{ key, input }, std::move(action));
    }

    void
    bind(SDL_Keycode key, std::initializer_list<KeyInput> inputs, InputAction action)
    {
        for (auto input : inputs)
        {
            bindings_.emplace(InputActionTrigger{ key, input }, action);
        }
    }

    void
    unbind(SDL_Keycode key, KeyInput input)
    {
        bindings_.erase(InputActionTrigger{ key, input });
    }

    template <class T>
    void
    bind(SDL_Keycode key, KeyInput input, void (T::*method)(), T *object)
    {
        bind(key, input, [object, method] { (object->*method)(); });
    }

    template <class T>
    void
    bind(SDL_Keycode key, KeyInput input, void (T::*method)(), std::shared_ptr<T> object)
    {
        bind(key,
             input,
             [weak = std::weak_ptr<T>(std::move(object)), method]
        {
            if (auto obj = weak.lock())
            {
                (obj.get()->*method)();
            }
            else
            {
                throw std::runtime_error("Input binding target expired");
            }
        });
    }

    template <class Method, class Object>
    void
    bind(SDL_Keycode key, std::initializer_list<KeyInput> inputs, Method method, Object object)
    {
        for (auto input : inputs)
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
    }
};

} // namespace ome::input
