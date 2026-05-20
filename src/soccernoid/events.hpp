#pragma once

#include "oh-my-engine/events.hpp"

namespace soccernoid {

struct ProjectileSpawned
{
};

struct ProjectileDespawned
{
};

struct PlayerDefeated
{
};

using EventBus = ome::EventBus<ProjectileSpawned, ProjectileDespawned, PlayerDefeated>;

} // namespace soccernoid
