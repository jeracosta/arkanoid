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
    // Point light at the fire pit lights nearby ground and characters
    class FirePointLightNode_ : public ome::LightNode
    {
      private:
        static ome::PointLight
        make_point_light_()
        {
            auto light                  = ome::PointLight{};
            light.ambient               = ome::Color::rgb(0.0f, 0.0f, 0.0f);
            light.diffuse               = ome::Color::rgb(1.0f, 0.55f, 0.12f);
            light.specular              = ome::Color::rgb(1.0f, 0.75f, 0.35f);
            light.constant_attenuation  = 1.0f;
            light.linear_attenuation    = 0.06f;
            light.quadratic_attenuation = 0.12f;
            return light;
        }

      public:
        FirePointLightNode_()
            : ome::LightNode(make_point_light_())
        {
            update_transform<ome::Space::Local>([](auto &t) { t.position = { 0.0f, 0.55f, 0.0f }; });
        }
    };

    class ParticlesNode_ : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {

            .initial_position = { ome::Box::from_size({ 0.5f, 0.0f, 0.5f }), rng },

            .initial_velocity
            = { ome::Box::from_bounds({ -0.5f, 1.2f, -0.5f }, { 0.5f, 2.8f, 0.5f }), rng },

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
                { 0.00f, 0.05f },
                { 0.10f, 0.20f },
                { 0.30f, 0.35f },
                { 0.50f, 0.30f },
                { 0.70f, 0.20f },
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
        emplace_child<FirePointLightNode_>().rename("FireLight");
        add_child(particles_).rename("FireParticles");
    }
};

} // namespace soccernoid
