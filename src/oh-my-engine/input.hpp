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

namespace ome::input {

enum class KeyInput : uint8_t
{
    Press,
    Repeat,
    Release,
};

using KeyInputAction = std::function<void()>;

struct KeyInputActionTrigger
{
    SDL_Keycode key;
    KeyInput    input;

    constexpr auto
    operator<=>(const KeyInputActionTrigger &) const
        = default;
};

class KeyboardInputMapper
{
  private:
    std::flat_multimap<KeyInputActionTrigger, KeyInputAction> bindings_;

    auto
    actions_for_(KeyInputActionTrigger trigger) const
    {
        auto [begin, end] = bindings_.equal_range(trigger);
        return std::ranges::subrange(begin, end) | std::views::values;
    }

  public:
    void
    bind(SDL_Keycode key, KeyInput input, KeyInputAction action)
    {
        bindings_.emplace(KeyInputActionTrigger{ key, input }, std::move(action));
    }

    void
    bind(SDL_Keycode key, std::initializer_list<KeyInput> inputs, KeyInputAction action)
    {
        for (auto input : inputs)
        {
            bindings_.emplace(KeyInputActionTrigger{ key, input }, action);
        }
    }

    void
    unbind(SDL_Keycode key, KeyInput input)
    {
        bindings_.erase(KeyInputActionTrigger{ key, input });
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

struct MouseMotionInput
{
    glm::vec2 delta;
    glm::vec2 position;
};

using MouseMotionInputAction = std::function<void(const MouseMotionInput &)>;

class MouseMotionInputMapper
{
  private:
    std::vector<MouseMotionInputAction> actions_;

  public:
    void
    bind(MouseMotionInputAction action)
    {
        actions_.emplace_back(std::move(action));
    }

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

        for (const auto &action : actions_)
        {
            action(input);
        }
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
