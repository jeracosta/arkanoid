#pragma once
#include "frame_context.hpp"
#include <SDL2/SDL.h>
#include <actions.hpp>
#include <cstdint>
#include <flat_map>
#include <functional>
#include <optional>
#include <ranges>

class InputAction
{
  public:
    enum class Trigger : uint8_t
    {
        Press,
        Sustain,
        Release,
    };

    struct Threshold
    {
        enum class Mode : uint8_t
        {
            Absolute,
            Relative,
        };

        float value;
        Mode  mode;
    };

    struct Update
    {
        float value;
    };

    using Handler = std::function<void(Trigger trigger, float value, const FrameContext &context)>;

    struct Config
    {
        Threshold threshold = { .value = 0.5f, .mode = Threshold::Mode::Absolute };

        Handler handler = {};
    };

  private:
    Config config_;
    float  value_  = 0.0f;
    bool   active_ = false;

    float
    measured_value_(Update update) const
    {
        using enum Threshold::Mode;

        switch (config_.threshold.mode)
        {
        case Absolute:
            return update.value;
        case Relative:
            return update.value - value_;
        }
    }

    bool
    is_active_(float value) const
    {
        return value >= config_.threshold.value;
    }

    std::optional<Trigger>
    get_trigger_(Update update) const
    {
        bool is_active  = is_active_(measured_value_(update));
        bool was_active = active_;

        if (is_active && !was_active)
        {
            return Trigger::Press;
        }

        if (is_active && was_active)
        {
            return Trigger::Sustain;
        }

        if (!is_active && was_active)
        {
            return Trigger::Release;
        }

        return std::nullopt;
    }

  public:
    InputAction(Config config)
        : config_(config)
    {
    }

    InputAction(Handler handler)
        : config_({ .handler = std::move(handler) })
    {
    }

    InputAction(std::function<void(Trigger)> handler)
        : InputAction([h = std::move(handler)](auto trigger, auto, auto &) { h(trigger); })
    {
    }

    void
    update(Update update, FrameContext context)
    {
        auto trigger = get_trigger_(update);

        if (config_.handler && trigger)
        {
            config_.handler(*trigger, measured_value_(update), context);
        }

        value_  = update.value;
        active_ = is_active_(update.value);
    }
};

enum class InputDevice
{
    Keyboard,
    Mouse,
};

struct InputChannel
{
    struct BoolBased
    {
        using Identifier = std::size_t;

        InputDevice device;

        Identifier id;

        template <typename T>
        static constexpr T
        value_cast(bool value)
        {
            if constexpr (std::is_same_v<T, float>)
            {
                return value ? std::numeric_limits<float>::infinity()
                             : -std::numeric_limits<float>::infinity();
            }
        }

        bool
        operator<(const InputChannel::BoolBased &other) const
        {
            return std::tie(device, id) < std::tie(other.device, other.id);
        }
    };

    static InputChannel::BoolBased
    Key(SDL_Scancode scan_code)
    {
        return { .device = InputDevice::Keyboard, .id = scan_code };
    }

    static InputChannel::BoolBased
    MouseButton(uint8_t button_code)
    {
        return { .device = InputDevice::Mouse, .id = button_code };
    }
};

class InputMapper
{
  private:
    std::vector<InputAction> actions_; // indexed by ActionId_ value

    using ActionId_ = std::size_t;

    std::flat_multimap<InputChannel::BoolBased, ActionId_> channels_to_action_ids_;

    struct ParsedEvent
    {
        InputChannel::BoolBased channel;
        InputAction::Update     update;
    };

    std::optional<ParsedEvent>
    parse_(const SDL_Event &event)
    {

        InputAction::Update on_up   = { InputChannel::BoolBased::value_cast<float>(true) },
                            on_down = { InputChannel::BoolBased::value_cast<float>(false) };

        switch (event.type)
        {
        case SDL_KEYDOWN:
            return { { InputChannel::Key(event.key.keysym.scancode), on_down } };
        case SDL_KEYUP:
            return { { InputChannel::Key(event.key.keysym.scancode), on_up } };
        case SDL_MOUSEBUTTONDOWN:
            return { { InputChannel::MouseButton(event.button.button), on_down } };
        case SDL_MOUSEBUTTONUP:
            return { { InputChannel::MouseButton(event.button.button), on_up } };
        default:
            return std::nullopt; // ignore other events
        }
    }

    auto
    binded_actions_(InputChannel::BoolBased channel)
    {
        auto [begin, end] = channels_to_action_ids_.equal_range(channel);

        return std::ranges::subrange(begin, end)
               | std::views::transform([this](auto const &pair) -> InputAction &
        { return actions_[pair.second]; });
    }

  public:
    using Binding = std::pair<InputAction, std::initializer_list<InputChannel::BoolBased>>;

    InputMapper(std::initializer_list<Binding> bindings)
    {
        for (auto const &[action, inputs] : bindings)
        {
            auto action_id = actions_.size();
            actions_.push_back(std::move(action));

            auto to_pair = [action_id](auto input) { return std::make_pair(input, action_id); };

            channels_to_action_ids_.insert_range(inputs | std::views::transform(to_pair));
        }
    }

    void
    handle(const SDL_Event &raw_event, FrameContext context)
    {
        if (auto event = parse_(raw_event))
        {
            for (auto &action : binded_actions_(event->channel))
            {
                action.update(event->update, context);
            }
        };
    }
};
