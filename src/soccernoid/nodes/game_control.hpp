#pragma once

#include <format>
#include <imgui.h>

#include "soccernoid/events.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

// Tracks game-wide state.
class GameControlNode : public SoccernoidNode<>
{
  private:
    int   projectile_count_    = 0;
    int   total_spawned_count_ = 0;
    int   score_               = 0;
    int   level_number_        = 1;
    float level_start_time_    = 0.0f;
    bool  game_over_           = false;
    bool  resetting_           = false;

    void
    reset_()
    {
        resetting_ = true;
        game()->schedule([this]
        {
            projectile_count_    = 0;
            total_spawned_count_ = 0;
            score_               = 0;
            level_start_time_    = game()->time.elapsed();
            game_over_           = false;
            resetting_           = false;
        });
    }

    void
    draw_hud_()
    {
        constexpr auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
                               | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
                               | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs
                               | ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(ImVec2{ 16.0f, 16.0f }, ImGuiCond_Always);
        ImGui::Begin("##game-hud", nullptr, flags);
        ImGui::Text("Level: %d", level_number_);
        ImGui::Text("Time: %.1f", game()->time.elapsed() - level_start_time_);
        ImGui::Text("Points: %d", score_);
        ImGui::End();
    }

    void
    on_score_awarded_(const ScoreAwarded &score)
    {
        score_ += score.points;
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
        auto &events      = game()->events;
        level_start_time_ = game()->time.elapsed();
        hold(events.bind(&GameControlNode::on_projectile_spawned_, this));
        hold(events.bind(&GameControlNode::on_projectile_despawned_, this));
        hold(events.bind(&GameControlNode::on_score_awarded_, this));
        hold(events.bind(&GameControlNode::on_goal_hit_, this));
        hold(game()->input.bind(Action::Reset, [this] { reset_(); }));
    }

    void
    on_tick_() override
    {
        draw_hud_();
    }
};

} // namespace soccernoid
