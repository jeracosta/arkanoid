#include <memory>

#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/spline.hpp"

namespace soccernoid {

class FireNode : public ome::TransformNode
{
  private:
    class ParticlesNode_ : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleBlueprint blueprint_ = {
            .color  = ome::SplineCurve<ome::Color>(ome::SplineCurve<ome::Color>::catmull_rom({
                { 0.00f, ome::Color::rgba(1.0f, 1.0f, 1.0f, 1.0f) },
                { 0.10f, ome::Color::rgba(1.0f, 1.0f, 1.0f, 1.0f) },
                { 0.20f, ome::Color::rgba(1.0f, 1.0f, 0.8f, 1.0f) },
                { 0.35f, ome::Color::rgba(1.0f, 0.7f, 0.2f, 1.0f) },
                { 0.50f, ome::Color::rgba(1.0f, 0.3f, 0.0f, 1.0f) },
                { 0.65f, ome::Color::rgba(0.8f, 0.1f, 0.0f, 1.0f) },
                { 0.80f, ome::Color::rgba(0.4f, 0.0f, 0.0f, 0.6f) },
                { 0.90f, ome::Color::rgba(0.1f, 0.0f, 0.0f, 0.2f) },
                { 1.00f, ome::Color::rgba(0.0f, 0.0f, 0.0f, 0.0f) },
            })),
            .scale  = ome::SplineCurve<float>(ome::SplineCurve<float>::catmull_rom({
                { 0.00f, 0.05f },
                { 0.10f, 0.20f },
                { 0.30f, 0.35f },
                { 0.50f, 0.30f },
                { 0.70f, 0.20f },
                { 1.00f, 0.00f },
            })),
            .origin           = ome::math::Box<3, float>({-0.3f, 0.0f, -0.3f}, {0.3f, 0.0f, 0.3f}),
            .initial_velocity = ome::math::Box<3, float>({-0.5f, 1.2f, -0.5f}, {0.5f, 2.8f, 0.5f}),
            .acceleration     = {},
            .angular_speed = ome::InterpolationCurve<float>{ 5.0f, 0.0f },
            .time_to_live  = 2.0f,
        };

        static inline const ome::ParticleEmitterNode::Configuration config_ = {
            .particle_blueprint       = blueprint_,
            .emissions_per_time_unit_ = 300,
        };

      public:
        ParticlesNode_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<ParticlesNode_> particles_ = std::make_shared<ParticlesNode_>();

  public:
    explicit FireNode(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("FireParticles").up();
    }
};

} // namespace soccernoid
