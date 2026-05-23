#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/sphere.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/nodes/light_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/spline.hpp"
#include "soccernoid/nodes/mixins/distance_culled.hpp"
#include "soccernoid/nodes/mixins/falling.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

class PlayerNode;
class MapNode;

class ProjectileNode : public SoccernoidNode<DistanceCulled<Falling<ome::KinematicNode>>>
{
  public:
    struct Configuration
    {
        float radius          = 0.10f;
        float elasticity      = 0.98f;
        float speed_threshold = 0.1f;
    };

  private:
    using Base_ = SoccernoidNode<DistanceCulled<Falling<ome::KinematicNode>>>;

    Configuration config_;

    PlayerNode *player_;
    MapNode    *map_;

    ome::HitboxNode *hitbox_;

    // when a projectile goes of bounds towards the forward end of the map, it wraps around
    bool wrapped_ = false;

    static constexpr ome::Vec3f spawn_position = { 0.0f, 7.0f, -3.0f };

    ome::Vec3f
    position_in_map_();

    ome::Vec3f
    force_();

    void
    bounce_from_(ome::HitboxNode &other);

  public:
    class HitboxNode : public ome::HitboxNode
    {
      public:
        HitboxNode(const Configuration &config)
            : ome::HitboxNode({ config.radius * 2 })
        {
        }

        void
        on_collision_(ome::HitboxNode &other) override;
    };

    class LightNode : public ome::LightNode<ome::PointLight>
    {
      public:
        static const inline auto light = ome::PointLight {
            .color = {
                .ambient  = ome::Color::rgb(0.0f, 0.0f, 0.0f),
                .diffuse  = ome::Color::rgb(0.4f, 1.0f, 0.5f),
                .specular = ome::Color::rgb(0.1f, 1.0f, 0.2f),
            },
            .attenuation = {
                .constant  = 0.00f,
                .linear    = 0.00f,
                .quadratic = 0.50f,
            }
        };

        LightNode()
            : ome::LightNode<ome::PointLight>(light, 2)
        {
            position({ 0.0f, 0.05f, 0.0f });
        }
    };

    class GlowParticlesNode : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {
            .initial_velocity = { ome::math::Sphere<3>({}, 3.0f), rng },

            .time_to_live = 0.1f,

            .color = ome::Interpolation{ ome::Color::white(), colors.projectile },

            .scale = ome::Spline<float>::catmull_rom({
                { 0.00f, 0.05f },
                { 0.50f, 0.20f },
                { 1.00f, 0.00f },
            }),

            .blend_mode = ome::BlendMode::additive(),
        };

        static inline const ome::ParticleEmitterNode::Settings settings_ = {
            .particle_blueprint = scheme_,
            .trigger_rate       = 1000,
        };

      public:
        GlowParticlesNode()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

    class TraceParticlesNode : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {
            .initial_position = { ome::math::Sphere<3>({ 0 }, 0.15f), rng },

            .initial_velocity = { ome::Box::from_size(0.75f), rng },

            .time_to_live = 1.0f,

            .color = ome::Interpolation{ ome::Color::white(), colors.projectile },

            .scale = ome::Interpolation{ 0.075f, 0.0f },
        };

        static inline const ome::ParticleEmitterNode::Settings settings_ = {
            .particle_blueprint = scheme_,
            .trigger_rate       = 300,
        };

      public:
        TraceParticlesNode()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

    ProjectileNode(Configuration config)
        : config_(std::move(config))
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = spawn_position; });

        emplace_child<LightNode>().rename("GlowLight");
        emplace_child<GlowParticlesNode>().rename("GlowParticles");
        emplace_child<TraceParticlesNode>().rename("TraceParticles");

        auto &hitbox = emplace_child<HitboxNode>(config_);
        hitbox.rename("Hitbox");
        hitbox_ = &hitbox;
    }

    ProjectileNode()
        : ProjectileNode(Configuration{})
    {
    }

    void
    on_tick_() override;

    void
    on_mount_() override;

    void
    on_unmount_() override;
};

} // namespace soccernoid
