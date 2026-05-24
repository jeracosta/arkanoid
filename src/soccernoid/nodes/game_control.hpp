#pragma once

#include <format>

#include "soccernoid/events.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

// Tracks game-wide state.
class GameControlNode : public SoccernoidNode<>
{
  private:
    int  projectile_count_    = 0;
    int  total_spawned_count_ = 0;
    bool game_over_           = false;
    bool resetting_           = false;

    void
    reset_()
    {
        resetting_ = true;
        game()->schedule([this]
        {
            projectile_count_    = 0;
            total_spawned_count_ = 0;
            game_over_           = false;
            resetting_           = false;
        });
    }

    void
    on_projectile_spawned_(const ProjectileSpawned &)
    {
        ++projectile_count_;
        ++total_spawned_count_;
        log(std::format("Projectile spawned (live: {})", projectile_count_));
    }

    void
    on_projectile_despawned_(const ProjectileDespawned &)
    {
        --projectile_count_;
        log(std::format("Projectile despawned (live: {})", projectile_count_));

        if (projectile_count_ == 0 && total_spawned_count_ > 0 && !game_over_ && !resetting_)
        {
            game_over_ = true;
            log("No projectiles left — player defeated");
            game()->events.emit(PlayerDefeated{});
        }
    }

    void
    on_goal_hit_(const GoalHit &)
    {
        if (game_over_)
        {
            return;
        }

        game_over_ = true;
        log("Goal hit — player victorious!");
        game()->events.emit(PlayerVictorious{});
    }

  public:
    void
    on_mount_() override
    {
        auto &events = game()->events;
        hold(events.bind(&GameControlNode::on_projectile_spawned_, this));
        hold(events.bind(&GameControlNode::on_projectile_despawned_, this));
        hold(events.bind(&GameControlNode::on_goal_hit_, this));
        hold(game()->input.bind(Action::Reset, [this] { reset_(); }));
    }
};

} // namespace soccernoid
