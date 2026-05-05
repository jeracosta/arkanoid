#pragma once

#include "oh-my-engine/input.hpp"

// Definition of the Action enum for the soccernoid game.
enum class ome::input::Action
{
    Quit,
    TogglePause,
    ToggleFullscreen,
    ChangeView,
    SummonBalls,
    JiggleBalls,
    PrintTree,
    TimeSpeedUp,
    TimeSpeedDown,
    Reset,
    CameraForward,
    CameraBackward,
    CameraUp,
    CameraDown,
    CameraLeft,
    CameraRight,
    CameraSprint,
    CameraFovUp,
    CameraFovDown,
};

namespace soccernoid {

using Action = ome::input::Action;

inline void
configure_default_controls(ome::input::InputMapper *input_mapper)
{
    using namespace ome::input;
    using enum KeyInput;
    using enum Action;

    struct BindingSpec
    {
        Action                action;
        SDL_Keycode           key;
        std::vector<KeyInput> key_inputs;
    };

    // clang-format off

    auto specs = std::to_array<BindingSpec>({
        { Quit,             SDLK_ESCAPE,  { Release }       },
        { ToggleFullscreen, SDLK_F11,     { Press }         },
        { TogglePause,      SDLK_p,       { Press }         },
        { ChangeView,       SDLK_TAB,     { Press }         },
        { SummonBalls,      SDLK_SPACE,   { Press, Repeat } },
        { JiggleBalls,      SDLK_RETURN,  { Press, Repeat } },
        { Reset,            SDLK_r,       { Press }         },
        { PrintTree,        SDLK_i,       { Press }         },
        { TimeSpeedUp,      SDLK_PLUS,    { Press, Repeat } },
        { TimeSpeedDown,    SDLK_MINUS,   { Press, Repeat } },
        { CameraForward,    SDLK_w,       { Press }         },
        { CameraBackward,   SDLK_s,       { Press }         },
        { CameraLeft,       SDLK_a,       { Press }         },
        { CameraRight,      SDLK_d,       { Press }         },
        { CameraUp,         SDLK_SPACE,   { Press }         },
        { CameraDown,       SDLK_LCTRL,   { Press }         },
        { CameraSprint,     SDLK_LSHIFT,  { Press }         },
    });

    // clang-format on

    for (auto &[action, key, key_inputs] : specs)
    {
        for (auto &key_input : key_inputs)
        {
            input_mapper->bind({ key, key_input }, action);
        }
    }
}

}; // namespace soccernoid
