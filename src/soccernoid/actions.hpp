#pragma once

#include "oh-my-engine/input.hpp"

// Definition of the Action enum for the soccernoid game.
enum class ome::input::Action
{
    Quit,
    Pause,
    ToggleFullscreen,
    ChangeView,
    JiggleBalls,
    PrintInfo,
    SpeedUp,
    SpeedDown,
    Reset,
    MoveForward,
    MoveBackward,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
};

namespace soccernoid {
using Action = ome::input::Action;
};
