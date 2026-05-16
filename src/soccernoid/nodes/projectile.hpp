#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/game.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/sphere.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/spline.hpp"
#include "soccernoid/nodes/mixins/distance_culled.hpp"
#include "soccernoid/nodes/mixins/falling.hpp"

namespace soccernoid {

class ProjectileNode : public DistanceCulled<Falling<ome::KinematicNode>>
{
  private:
    using Base_ = DistanceCulled<Falling<ome::KinematicNode>>;

    static constexpr float radius_          = 0.10f;
    static constexpr float elasticity_      = 0.90f; // 1.0f = perfectly elastic, 0.0f = inelastic
    static constexpr float speed_threshold_ = 0.1f;

    static constexpr ome::Vec3f spawn_position = { 0.0f, 7.0f, -3.0f };

    class HitboxNode_ : public ome::HitboxNode
    {
      public:
        HitboxNode_()
            : ome::HitboxNode({ radius_ * 2 })
        {
        }

        void
        on_collision_(ome::HitboxNode &other) override
        {
            log(std::format("Collided with {} ({})", other.name(), other.default_name()),
                ome::LogLevel::Debug);

            auto hitbox        = this->hitbox<ome::Space::World>();
            auto others_hitbox = other.hitbox<ome::Space::World>();

            auto *parent = static_cast<KinematicNode *>(this->parent());

            parent->update_kinematic<ome::Space::Local>([&](ome::KinematicComponent &kinematic)
            {
                constexpr float epsilon = 1e-4f;

                auto overlap          = overlap_depth(hitbox, others_hitbox);
                auto penetration_axis = std::ranges::min_element(overlap) - overlap.begin();

                if (overlap[penetration_axis] < epsilon)
                {
                    return;
                }

                bool is_negative = (hitbox.center()[penetration_axis]
                                    < others_hitbox.center()[penetration_axis]);

                ome::Vec3f normal;
                normal[penetration_axis] = is_negative ? -1.0f : 1.0f;

                auto normal_velocity = projection(kinematic.velocity, normal);
                auto normal_speed    = norm(normal_velocity);

                if (normal_speed < speed_threshold_)
                {
                    kinematic.velocity -= normal_velocity;
                }
                else
                {
                    kinematic.velocity -= (1.0f + elasticity_) * normal_velocity;
                }

                auto correction = normal * (overlap[penetration_axis] + epsilon);

                parent->update_transform<ome::Space::Local>([&](auto &transform)
                { transform.position += correction; });
            });
        }
    };

    class GlowParticlesNode_ : public ome::ParticleEmitterNode
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
        GlowParticlesNode_()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

    class TraceParticlesNode_ : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {
            .initial_position = { ome::math::Sphere<3>({ 0 }, 0.15f), rng },

            .initial_velocity = { ome::Box::from_size(0.75f), rng },

            .time_to_live = 1.0f,

            .color = ome::Interpolation{ ome::Color::white(), colors.projectile },

            .scale = ome::Interpolation{ 0.075f, 0.0f },

            .blend_mode = ome::BlendMode::additive(),
        };

        static inline const ome::ParticleEmitterNode::Settings settings_ = {
            .particle_blueprint = scheme_,
            .trigger_rate       = 300,
        };

      public:
        TraceParticlesNode_()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

  public:
    ProjectileNode()
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = spawn_position; });

        emplace_child<GlowParticlesNode_>().rename("GlowParticles");
        emplace_child<TraceParticlesNode_>().rename("TraceParticles");
        emplace_child<HitboxNode_>().rename("Hitbox");
    }

    void
    on_tick_() override
    {
        Base_::on_tick_();
    }
};

} // namespace soccernoid
