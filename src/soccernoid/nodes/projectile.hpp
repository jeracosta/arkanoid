#pragma once

#include <memory>
#include <cstdlib>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/box.hpp"
#include "oh-my-engine/color.hpp"
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

namespace soccernoid {

class ProjectileNode : public DistanceCulled<Falling<ome::KinematicNode>>
{
  private:
    using Base_ = DistanceCulled<Falling<ome::KinematicNode>>;

    float radius_          = 0.10f; // futsal size 4 ball: circumference 62-64cm, radius ~0.10m
    float elasticity_      = 0.90f;
    float speed_threshold_ = 0.1f;

    static constexpr ome::Vec3f spawn_position = { 0.0f, 7.0f, -3.0f };

    ome::Hitbox hitbox_{ { -0.10f, -0.10f, -0.10f }, { 0.10f, 0.10f, 0.10f } };

    // Point light parented to the projectile so a colored pool follows the ball (GL_LIGHT2).
    class ProjectilePointLightNode_ : public ome::LightNode
    {
      private:
        static std::unique_ptr<ome::PointLight>
        make_point_light_()
        {
            auto light = std::make_unique<ome::PointLight>(GL_LIGHT2);
            light->ambient   = ome::Color::rgb(0.0f, 0.0f, 0.0f);
            light->diffuse   = ome::Color::rgb(1.0f, 0.45f, 0.95f);
            light->specular  = ome::Color::rgb(0.9f, 0.75f, 1.0f);
            light->constant_attenuation  = 1.0f;
            light->linear_attenuation      = 0.12f;
            light->quadratic_attenuation   = 0.28f;
            return light;
        }

      public:
        ProjectilePointLightNode_()
            : ome::LightNode(make_point_light_())
        {
            auto transform     = local_transform();
            transform.position = { 0.0f, 0.2f, 0.0f };
            set_local_transform(transform);
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

            .initial_velocity = { ome::Box(0.75f), rng },

            .time_to_live = 1.0f,

            .color = ome::Interpolation{ ome::Color::white(), colors.projectile },

            .scale = ome::Interpolation{ 0.075f, 0.0f },
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

        extending(*this).add<ProjectilePointLightNode_>().named("GlowLight").up();
        extending(*this).add<GlowParticlesNode_>().named("GlowParticles");
        extending(*this).add<TraceParticlesNode_>().named("TraceParticles");
    }

    void
    on_tick_() override
    {
        Base_::on_tick_();
        bounce_();
    }

  private:
    void
    bounce_()
    {
        if (transform<ome::Space::World>().position[1] <= radius_)
        {
            update_transform<ome::Space::Local>([&](auto &t) { t.position[1] = radius_; });

            float fall_speed = dot(velocity(), ome::up);
            if (fall_speed >= 0.0f)
            {
                return;
            }

            if (std::abs(fall_speed) < speed_threshold_)
            {
                set_velocity({ velocity()[0], 0.0f, velocity()[2] });
                schedule_unmount();
                return;
            }

            set_velocity({ velocity()[0], -fall_speed * elasticity_, velocity()[2] });
            return;
        }
    }
};

} // namespace soccernoid
