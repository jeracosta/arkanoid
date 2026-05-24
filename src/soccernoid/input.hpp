#pragma once

#include <SDL_keycode.h>

#include "oh-my-engine/input.hpp"

// Definition of the Action enum for the soccernoid game.
enum class ome::input::Action
{
    Quit,
    TogglePause,
    ToggleFullscreen,
    ChangeView,
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
    PlayerLeft,
    PlayerRight,
    PlayerJump,
    PlayerShoot,
    ToggleHud,
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
        { Quit,             SDLK_q,        { Release }      },
        { ToggleFullscreen, SDLK_F11,      { Press }        },
        { TogglePause,      SDLK_p,        { Press }        },
        { ChangeView,       SDLK_v,        { Press }        },
        { Reset,            SDLK_r,        { Press, Repeat }},
        { PrintTree,        SDLK_i,        { Press }        },
        { TimeSpeedUp,      SDLK_PLUS,     { Press, Repeat }},
        { TimeSpeedUp,      SDLK_EQUALS,   { Press, Repeat }},
        { TimeSpeedUp,      SDLK_KP_PLUS,  { Press, Repeat }},
        { TimeSpeedDown,    SDLK_MINUS,    { Press, Repeat }},
        { TimeSpeedDown,    SDLK_KP_MINUS, { Press, Repeat }},
        { CameraForward,    SDLK_w,        { Press }        },
        { CameraBackward,   SDLK_s,        { Press }        },
        { CameraLeft,       SDLK_a,        { Press }        },
        { CameraRight,      SDLK_d,        { Press }        },
        { CameraUp,         SDLK_SPACE,    { Press }        },
        { CameraDown,       SDLK_LCTRL,    { Press }        },
        { CameraSprint,     SDLK_LSHIFT,   { Press }        },
        { PlayerLeft,       SDLK_LEFT,     { Press }        },
        { PlayerRight,      SDLK_RIGHT,    { Press }        },
        { PlayerJump,       SDLK_UP,       { Press }        },
        { PlayerShoot,      SDLK_UP,       { Press, Repeat }},
        { ToggleHud,        SDLK_ESCAPE,   { Press }        },
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
