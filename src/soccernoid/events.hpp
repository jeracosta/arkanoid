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

struct ObstacleDestroyed
{
    int projectiles_spawned;
};

struct GoalHit
{
};

struct PlayerVictorious
{
};

struct AppTerminated
{
};

using GameEventBus = ome::EventBus<ProjectileSpawned,
                                   ProjectileDespawned,
                                   PlayerDefeated,
                                   ObstacleDestroyed,
                                   GoalHit,
                                   PlayerVictorious,
                                   AppTerminated>;

} // namespace soccernoid
