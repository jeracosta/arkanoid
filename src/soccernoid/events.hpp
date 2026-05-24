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

struct ScoreAwarded
{
    int points;
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
                                   ScoreAwarded,
                                   GoalHit,
                                   PlayerVictorious,
                                   AppTerminated>;

} // namespace soccernoid
