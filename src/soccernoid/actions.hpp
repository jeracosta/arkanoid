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
    PrintInfo,
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
};

namespace soccernoid {
using Action = ome::input::Action;
};
