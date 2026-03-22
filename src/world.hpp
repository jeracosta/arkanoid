#pragma once

#include "application.hpp"
#include <optional>

// Global runtime context. Provides access to core game data (e.g. delta time) for all game logic.
extern const std::optional<const Application::RuntimeContext> &WORLD;
