#pragma once

#include "chronometer.hpp"
#include <chrono>
#include <functional>

struct FrameContext
{

    using TimeUnit = std::chrono::duration<float, std::ratio<1>>; // Seconds as float

    // Time measured at the start of the current frame.
    const Chronometer<TimeUnit>::Reading &time;

    // Triggers a graceful shutdown of the application at the end of the current frame.
    std::function<void()> stop;
};
