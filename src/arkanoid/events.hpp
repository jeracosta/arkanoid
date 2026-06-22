#pragma once

#include "oh-my-engine/events.hpp"

namespace arkanoid {

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

} // namespace arkanoid
