#include <memory>

#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace soccernoid {

class CometNode : public ome::TransformNode
{
  private:
    struct
    {
        ome::Vec3f       center      = { 0.0f };
        float            radius      = 25.0f;
        float            speed       = 0.05f;
        ome::Orientation orientation = ome::Orientation{}.steer_pitch(0.5f);

    } movement_;

    class ParticlesNode_ : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleBlueprint blueprint_ = {
            .color
            = ome::InterpolationCurve<ome::Color>{
                ome::Color::rgba(1.0f, 1.0f, 1.0f, 1.0f),
                ome::Color::rgba(0.0f, 0.0f, 1.0f, 0.0f) },
            .scale            = ome::InterpolationCurve<float>{ 0.3f, 0.05f },
            .origin           = ome::Vec3f{ 0.0, 0.2, 0.0 },
            .initial_velocity = ome::math::Box<3, float>({ -0.5, -0.5, -0.5 }, { 0.5, 0.5, 0.5 }),
            .acceleration     = {},
            .angular_speed    = ome::InterpolationCurve<float>{ 0, 0 },
            .time_to_live     = 1.0,
        };

        static inline const ome::ParticleEmitterNode::Configuration config_ = {
            .particle_blueprint       = blueprint_,
            .emissions_per_time_unit_ = 200,
        };

      public:
        ParticlesNode_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<ParticlesNode_> particles_ = std::make_shared<ParticlesNode_>();

  public:
    CometNode()
        : TransformNode()
    {
        extending(*this).add(particles_).named("Particles").up();
    }

    void
    on_tick_() override
    {
        auto angle   = movement_.speed * Node::game()->time.elapsed();
        auto tangent = ome::Vec3f{ -std::sin(angle), 0.0f, std::cos(angle) };

        auto offset   = ome::Vec3f{ std::cos(angle), 0.0f, std::sin(angle) } * movement_.radius;
        auto position = movement_.center + movement_.orientation * offset;

        set_local_transform({ .position = position });

        particles_->blueprint()->acceleration = movement_.orientation * -tangent * 1.5f;
    }
};

} // namespace soccernoid
