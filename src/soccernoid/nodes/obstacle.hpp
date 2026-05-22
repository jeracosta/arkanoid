#pragma once

#include <format>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

class ObstacleNode : public SoccernoidNode<ome::HitboxNode>
{
  public:
    struct Configuration
    {
        ome::Vec3f position;
        ome::Vec3f size;
        int        life;
        int        projectile_spawn_count;
        ome::Color color;

        static Configuration
        defaults()
        {
            return {
                .position               = { 0.0f, 1.0f, -3.0f },
                .size                   = { 0.6f, 0.6f, 0.6f },
                .life                   = 3,
                .projectile_spawn_count = 2,
                .color                  = ome::Color::rgb(180, 80, 80),
            };
        }
    };

  private:
    Configuration config_;
    int           current_life_;

    static ome::Color
    lerp_color_(const ome::Color &a, const ome::Color &b, float t)
    {
        return a * (1.0f - t) + b * t;
    }

    void
    render_()
    {
        auto region = hitbox<ome::Space::World>();

        float damage_ratio = 1.0f - static_cast<float>(current_life_) / config_.life;
        auto  color        = lerp_color_(config_.color, ome::Color::rgb(60, 60, 60), damage_ratio);

        glColor(color);

        auto placeholder = ome::Texture::placeholder();

        using Bounds = ome::Rect::Bounds;
        ome::Sprite sprite{ placeholder, Bounds{ { 0.0f, 0.0f }, { 1.0f, 1.0f } } };

        ome::open_gl::BoxRenderTask{
            .world_region = region,
            .sprites      = { sprite, sprite, sprite, sprite, sprite, sprite },
        }();

        glColor(ome::Color::white());
    }

    void
    spawn_projectiles_(const ome::Vec3f &impact_velocity)
    {
        auto world_pos = transform<ome::Space::World>().position;

        auto base_dir = -impact_velocity;
        base_dir[1]   = 0.0f;

        float base_speed = norm(impact_velocity);
        if (norm(base_dir) > 0.001f)
        {
            base_dir = normalized(base_dir);
        }
        else
        {
            base_dir = ome::Vec3f{ 0.0f, 0.0f, 1.0f };
        }

        float speed = base_speed * 0.8f;

        for (int i = 0; i < config_.projectile_spawn_count; ++i)
        {
            float t            = static_cast<float>(i) / std::max(1, config_.projectile_spawn_count - 1);
            float spread_angle = (t - 0.5f) * 2.0f;

            ome::Vec3f horizontal_dir = {
                base_dir[0] * std::cos(spread_angle) - base_dir[2] * std::sin(spread_angle),
                0.0f,
                base_dir[0] * std::sin(spread_angle) + base_dir[2] * std::cos(spread_angle),
            };

            ome::Vec3f dir = normalized(horizontal_dir) * 0.9f + ome::up * 0.3f;

            auto spawn_pos = world_pos + normalized(horizontal_dir) * 1.2f + ome::up * 0.3f;
            auto velocity  = normalized(dir) * speed;

            game()->schedule([this, spawn_pos, velocity]
            {
                if (auto *level = parent())
                {
                    auto &projectile = level->emplace_child<ProjectileNode>();
                    projectile.rename(std::format("SpawnedProjectile_{}", reinterpret_cast<uintptr_t>(&projectile)));
                    projectile.update_transform<ome::Space::Local>([&](auto &t)
                    { t.position = spawn_pos; });
                    projectile.velocity(velocity);
                }
            });
        }

        game()->Events.emit(ObstacleDestroyed{ .projectiles_spawned = config_.projectile_spawn_count });
    }

  public:
    explicit ObstacleNode(const Configuration &config)
        : SoccernoidNode<ome::HitboxNode>(config.size),
          config_(config),
          current_life_(config_.life)
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = config_.position; });
    }

    void
    on_collision_(ome::HitboxNode &other) override
    {
        if (other.name().find("Hitbox") == std::string::npos)
        {
            return;
        }

        ome::Vec3f impact_velocity = { 0.0f, 0.0f, -1.0f };
        if (auto *projectile = dynamic_cast<ome::KinematicNode *>(other.parent()))
        {
            impact_velocity = projectile->kinematic<ome::Space::World>().velocity;
        }

        --current_life_;
        log(std::format("Obstacle hit! Life: {}/{}", current_life_, config_.life));

        if (current_life_ <= 0)
        {
            log("Obstacle destroyed! Spawning projectiles...");
            spawn_projectiles_(impact_velocity);
            schedule_unmount();
        }
    }

    void
    on_tick_() override
    {
        render_();
    }
};

} // namespace soccernoid
