#pragma once

#include <memory>

#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/sphere.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/nodes/light_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/spline.hpp"

namespace arkanoid {

class FireExplosionNode : public ome::KinematicNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {

            .initial_position = [] { return ome::Vec3f{ 0 }; },

            .initial_velocity = { ome::math::Sphere<3>({}, 15.0f), rng },

            .time_to_live = 0.4f,

            .color = ome::Spline<ome::Color>::catmull_rom({
                { 0.00f, ome::Color::white() },
                { 0.10f, ome::Color::rgba(1.0f, 1.0f, 0.8f, 1.0f) },
                { 0.30f, ome::Color::rgba(1.0f, 0.6f, 0.1f, 1.0f) },
                { 0.50f, ome::Color::rgba(1.0f, 0.2f, 0.0f, 1.0f) },
                { 0.70f, ome::Color::rgba(0.5f, 0.0f, 0.0f, 0.6f) },
                { 1.00f, ome::Color::rgba(0.0f, 0.0f, 0.0f, 0.0f) },
            }),

            .scale = ome::Spline<float>::catmull_rom({
                { 0.00f, 0.1f },
                { 0.20f, 0.5f },
                { 0.50f, 0.3f },
                { 1.00f, 0.0f },
            }),

            .blend_mode = ome::BlendMode::additive(),
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint    = blueprint_,
            .trigger_rate          = 1000,
            .particles_per_trigger = 500,
            .total_emissions_count = 1,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    class Light_ : public ome::LightNode<ome::PointLight>
    {
      private:
        ome::CurveProcess<float> intensity_{ std::make_shared<ome::Spline<float>>(
            ome::Spline<float>::catmull_rom({
                { 0.0f, 1.0f },
                { 0.1f, 0.3f },
                { 0.3f, 0.05f },
                { 1.0f, 0.0f },
            })) };

        static constexpr auto starting_light_ = ome::PointLight{
                    .color = {
                        .ambient  = ome::Color::rgb(0.0f, 0.0f, 0.0f),
                        .diffuse  = ome::Color::rgb(1.0f, 0.8f, 0.3f),
                        .specular = ome::Color::rgb(1.0f, 0.6f, 0.2f),
                    },
                    .attenuation = { 0.0f, 0.0f, 0.1f },
                };

      public:
        Light_()
            : ome::LightNode<ome::PointLight>(starting_light_, 3)
        {
        }

        void
        on_tick_() override
        {
            intensity_.update(game()->time.delta());
            auto intensity        = intensity_.value();
            light_.color.diffuse  = ome::Color::rgb(ome::Vec3f{ 1.0f, 0.8f, 0.3f } * intensity);
            light_.color.specular = ome::Color::rgb(ome::Vec3f{ 1.0f, 0.6f, 0.2f } * intensity);
            light_.attenuation.quadratic = 0.5f / (intensity + 0.01f);
        }
    };

  public:
    FireExplosionNode()
    {
        emplace_child<Particles_>().rename("Particles");
        emplace_child<Light_>().rename("Light");
    }

    void
    on_tick_() override
    {
        ome::KinematicNode::on_tick_();

        // Despawn once the effect is over; otherwise the node and its Light_ (which keeps
        // pushing lights every frame) would leak forever, since explosions live under the root.
        age_ += game()->time.delta();
        if (age_ >= lifetime_)
        {
            request_unmount();
        }
    }

  private:
    static constexpr float lifetime_ = 1.5f;
    float                  age_      = 0.0f;
};

} // namespace arkanoid
