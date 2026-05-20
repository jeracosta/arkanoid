#pragma once

#include <format>

#include "oh-my-engine/node.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/soccernoid.hpp"

namespace soccernoid {

// Tracks game-wide state. Currently: counts live projectiles and emits
// PlayerDefeated when the count drops to zero. Intended to live alongside the
// LevelNode under the root so it survives level swaps.
class GameControlNode : public ome::Node
{
  private:
    int projectile_count_ = 0;

    void
    on_projectile_spawned_(const ProjectileSpawned &)
    {
        ++projectile_count_;
        log(std::format("Projectile spawned (live: {})", projectile_count_));
    }

    void
    on_projectile_despawned_(const ProjectileDespawned &)
    {
        --projectile_count_;
        log(std::format("Projectile despawned (live: {})", projectile_count_));

        if (projectile_count_ == 0)
        {
            log("No projectiles left — player defeated");
            Soccernoid::from(*this).Events.emit(PlayerDefeated{});
        }
    }

  public:
    void
    on_mount_() override
    {
        auto &events = Soccernoid::from(*this).Events;
        hold(events.bind(&GameControlNode::on_projectile_spawned_, this));
        hold(events.bind(&GameControlNode::on_projectile_despawned_, this));
    }
};

} // namespace soccernoid
