#include <SDL2/SDL.h>
#include <actions.hpp>
#include <any>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <functional>
#include <ranges>
#include <utility>
#include <variant>

// TODO: remove

// Identifies high-level game intents triggered by input signals.
// Forward declaration only; users should define it.
// WARN: values must be sequence in [0, ACTION_COUNT_MAX); required for internal optimizations.
enum class Action : uint8_t;

// Maximum number of input actions supported.
// Keep small for performance.
#define ACTION_COUNT_MAX 64

using ActionMask = std::bitset<ACTION_COUNT_MAX>;

namespace input
{

enum class InputKind
{
    Button,
    Axis,
};

// Source of input signals, e.g. "the W key" or "the left mouse button".
class Source
{
  public:
    // Corresponds to input sources related by device and kind of input.
    // WARN: values must be sequence in [0, CATEGORY_COUNT).
    enum class Category : uint8_t
    {
        KeyboardKey,
        MouseButton,
        MouseAxis,
        Count_ // sentinel value; must be last
    };
    static constexpr std::size_t CATEGORY_COUNT = std::to_underlying(Category::Count_);

    using Identifier = uint8_t; // uniquely identifies a Source; should be compact for performance.

    Identifier id;

    static constexpr const Source Key(SDL_Scancode);

    static const Source LeftClick;
    static const Source RightClick;

    friend constexpr std::optional<Source>
    make_source(const SDL_Event);

    constexpr auto
    operator<=>(const Source &other) const
    {
        return id <=> other.id;
    }

  private:
    constexpr Source(auto category, auto code)
        : id(make_id_(category, code))
    {
    }

    // Identifies a Source from its category and a code that identifies it within its category.
    constexpr uint8_t
    make_id_(Category category, std::size_t code)
    {
        static constexpr std::size_t max_id = std::numeric_limits<Identifier>::max();

        auto           category_index = std::to_underlying(category);
        constexpr auto max_category   = max_id / CATEGORY_COUNT;

        auto code_displacement = std::to_underlying(category) * CATEGORY_COUNT;
        auto max_code          = max_id - code_displacement;

        // overflow checks
        assert(category_index <= max_category);
        assert(code <= max_code);

        return static_cast<Identifier>(code_displacement + code);
    }
};

constexpr const Source
Source::Key(SDL_Scancode s)
{
    if (s >= SDL_NUM_SCANCODES)
        throw "Invalid scan code";
    return { Category::KeyboardKey, s };
}

inline const Source Source::LeftClick{ Category::MouseButton, SDL_BUTTON_LEFT };

inline const Source Source::RightClick{ Category::MouseButton, SDL_BUTTON_RIGHT };

// Maps an SDL_Event to an input source, if supported.
constexpr std::optional<Source>
make_source(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        return Source::Key(event.key.keysym.scancode);
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        switch (event.button.button)
        {
        case SDL_BUTTON_LEFT:
            return Source::LeftClick;
        case SDL_BUTTON_RIGHT:
            return Source::RightClick;
        default:
            return std::nullopt; // unsupported mouse button
        }
    default:
        return std::nullopt; // unsupported event type
    }
}

// Type of input received from button-like sources.
enum class ButtonInput : uint8_t
{
    Press,
    Repeat,
    Release,
};

// Type of input received from axis-like sources.
using AxisInput = float; // e.g. mouse movement delta or joystick position.

using Input = std::variant<ButtonInput, AxisInput>;

template <Source::Category Category>
consteval auto
dummy_input()
{
    if constexpr (Category == Source::Category::KeyboardKey)
        return ButtonInput{};
    else if constexpr (Category == Source::Category::MouseButton)
        return ButtonInput{};
    else if constexpr (Category == Source::Category::MouseAxis)
        return AxisInput{};
    else
        static_assert([] { return false; }(), "Unspecified Source::Category signal kind");
}

template <Source::Category C>
using typeof_input = decltype(dummy_input<C>);

// Transforms an SDL_Event into its input representation.
// Must be called only for supported event types (see make_source).
constexpr Input
make_input(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
        return event.key.repeat ? ButtonInput::Repeat : ButtonInput::Press;
    case SDL_KEYUP:
        return ButtonInput::Release;
    case SDL_MOUSEBUTTONDOWN:
        return ButtonInput::Press;
    case SDL_MOUSEBUTTONUP:
        return ButtonInput::Release;
    default:
        assert(!"Unsupported event type for input mapping");
    }
}

struct ButtonInputHandler
{
    std::any context;
    using Function = void (*)(std::any, ButtonInput);

    Function on_press, on_repeat, on_release;

    Function &
    operator[](ButtonInput input)
    {
        switch (input)
        {
        case ButtonInput::Press:
            return on_press;
        case ButtonInput::Repeat:
            return on_repeat;
        case ButtonInput::Release:
            return on_release;
        default:
            assert(!"Invalid ButtonInput value");
        }
    }

    void
    operator()(const ButtonInput &input)
    {
        (*this)[input](context, input);
    }
};

struct AxisInputHandler; // TODO

// Maps input sources to binded handlers, and can call them in response to SDL events.
class Mapper
{
  private:
    std::flat_multimap<Source, ButtonInputHandler> button_mappings_;

  public:
    void
    bind(Source source, ButtonInput handler)
    {
        button_mappings_.emplace(source, std::move(handler));
    }

    void
    handle(const SDL_Event &event)
    {
        auto source = make_source(event);
        auto input  = make_input(event);

        if (!source)
        {
            return; // unsupported event left unhandled
        }

        auto handlers = [&]
        {
            auto [begin, end] = button_mappings_.equal_range(*source);
            return std::ranges::subrange(begin, end) | std::views::values;
        }();

        for (auto &&handler : handlers)
        {
            handler(input);
        };
    }
};

template <typename InputType>
class BindingBuilder;

template <>
class BindingBuilder<ButtonInput>
{
  public:
    Mapper  &mapper;
    std::any context;

    template <std::size_t N>
    class Step;

    Step<0> bind(std::function<void(std::any, const ButtonInput &)>);
};

template <>
class BindingBuilder<ButtonInput>::Step<0>
{
  private:
    using Builder = BindingBuilder<ButtonInput>;

    Builder &builder_;

    ButtonInputHandler handler_;

    Step(auto &builder, auto handler)
        : builder_(builder),
          handler_(std::move(handler))
    {
    }

    friend Builder;

  public:
    void
    to(std::initializer_list<Source> sources)
    {
        for (auto &source : sources)
        {
            builder_.mapper.bind(source, handler_);
        }
    }
};

inline BindingBuilder<ButtonInput>::Step<0>
BindingBuilder<ButtonInput>::bind(std::function<void(std::any, const ButtonInput &)> func)
{
    return Step<0>{
        *this,
        [func = std::move(func), ctx = context](const ButtonInput &input) { func(ctx, input); },
    };
}

// Class able to configure a mapper to bind inputs to handlers that refer to itself,
// thus processing inputs as part of its own logic.
// WARN: if Listener refs get invalidated after configuring bindings, the behavior is undefined.
// TODO: leverage shared_ptr and weak_ptr to allow safe invalidation of Listeners.
class Listener
{
  protected:
    Listener() = default; // abstract base class

  public:
    virtual ~Listener() = default;

    virtual void
    ConfigureInputBindings(BindingBuilder<ButtonInput> &builder)
        = 0;
};

} // namespace input
