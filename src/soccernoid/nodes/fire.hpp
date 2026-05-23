#include <cmath>
#include <memory>

#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/light_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/spline.hpp"

namespace soccernoid {

class FireNode : public ome::TransformNode
{
  private:
    class LightNode_ : public ome::LightNode<ome::PointLight>
    {
      private:
        static inline std::minstd_rand rng_{ std::random_device{}() };

        static inline std::uniform_real_distribution<float> phase_distribution_{
            0.0f, 2.0f * std::numbers::pi_v<float>
        };

        // used to prevent different fire lights from flickering in sync
        float flicker_phase_offset_ = phase_distribution_(rng_);

        void
        update_light_()
        {
            light_.attenuation = {
                .constant  = 1.0f,
                .linear    = 0.06f,
                .quadratic = 0.12f,
            };

            constexpr auto flicker_depth = 0.5f; // %

            const auto intensity = ome::Vec3f{ (1 - flicker_depth) + flicker_depth * flicker_() };

            auto diffuse  = ome::Vec3f(1.00f, 0.35f, 0.12f) * intensity;
            auto specular = ome::Vec3f(1.00f, 0.35f, 0.35f) * intensity;

            light_.color.diffuse  = ome::Color::rgb(diffuse);
            light_.color.specular = ome::Color::rgb(specular);
        }

        float
        flicker_()
        {
            const float time = this->game()->time.elapsed() + flicker_phase_offset_;

            float flicker = 0.0f;

            flicker += std::sin(time * 14.6f);
            flicker += std::sin(time * 23.4f);
            flicker += std::sin(time * 38.2f);
            flicker += std::sin(time * 11.8f);

            flicker *= 0.25f;                // [-1,1]
            flicker = flicker * 0.5f + 0.5f; // [0,1]

            return flicker;
        }

      public:
        LightNode_()
        {
            position({ 0.0f, 0.5f, 0.0f });
        }

        void
        on_tick_() override
        {
            update_light_();
        }
    };

    class ParticlesNode_ : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {

            .initial_position = { ome::Box::from_size({ 0.3f, 0.0f, 0.3f }), rng },

            .initial_velocity
            = { ome::Box::from_bounds({ -0.3f, 0.6f, -0.3f }, { 0.3f, 1.4f, 0.3f }), rng },

            .time_to_live = 2.0f,

            .color = ome::Spline<ome::Color>::catmull_rom({
                { 0.00f, ome::Color::rgba(1.0f, 1.0f, 1.0f, 1.0f) },
                { 0.10f, ome::Color::rgba(1.0f, 1.0f, 1.0f, 1.0f) },
                { 0.20f, ome::Color::rgba(1.0f, 1.0f, 0.8f, 1.0f) },
                { 0.35f, ome::Color::rgba(1.0f, 0.7f, 0.2f, 1.0f) },
                { 0.50f, ome::Color::rgba(1.0f, 0.3f, 0.0f, 1.0f) },
                { 0.65f, ome::Color::rgba(0.8f, 0.1f, 0.0f, 1.0f) },
                { 0.80f, ome::Color::rgba(0.4f, 0.0f, 0.0f, 0.6f) },
                { 0.90f, ome::Color::rgba(0.1f, 0.0f, 0.0f, 0.2f) },
                { 1.00f, ome::Color::rgba(0.0f, 0.0f, 0.0f, 0.0f) },
            }),

            .scale = ome::Spline<float>::catmull_rom({
                { 0.00f, 0.02f },
                { 0.10f, 0.08f },
                { 0.30f, 0.15f },
                { 0.50f, 0.12f },
                { 0.70f, 0.07f },
                { 1.00f, 0.00f },
            }),

            .blend_mode = ome::BlendMode::additive(),
        };

        static inline const ome::ParticleEmitterNode::Settings settings_ = {
            .particle_blueprint = scheme_,
            .trigger_rate       = 300,
        };

      public:
        ParticlesNode_()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

    std::shared_ptr<ParticlesNode_> particles_ = std::make_shared<ParticlesNode_>();

  public:
    explicit FireNode(ome::Vec3f position = { 0 })
        : TransformNode()
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = position; });
        emplace_child<LightNode_>().rename("FireLight");
        add_child(particles_).rename("FireParticles");
    }
};

} // namespace soccernoid
