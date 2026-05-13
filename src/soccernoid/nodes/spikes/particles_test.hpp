#pragma once

#include <memory>

#include "oh-my-engine/math/region.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace soccernoid::spikes {

// Cyan → white-transparent over 5 s, shrink to 0, no movement.
struct TestCyanParticles
{
    static inline const auto color_from    = ome::Color::rgba(0.0f, 1.0f, 1.0f, 1.0f);
    static inline const auto color_to      = ome::Color::rgba(1.0f, 1.0f, 1.0f, 0.0f);
    static constexpr float   scale_from   = 0.06f;
    static constexpr float   scale_to     = 0.0f;
    static constexpr float   time_to_live = 30.0f;
};

// -----------------------------------------------------------------------
// Sphere — radius 1 m
// -----------------------------------------------------------------------
class TestSphereEmitter : public ome::TransformNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {
            .initial_position = [] {
                return ome::math::sample_uniform(
                    ome::math::AnyRegion<3, float>(ome::math::Sphere<3, float>{ {}, 0.7f }),
                    rng);
            },
            .initial_velocity = ome::Vec3f{},
            .angular_speed    = 0.0f,
            .time_to_live     = TestCyanParticles::time_to_live,
            .acceleration     = ome::Vec3f{},
            .color            = ome::InterpolationCurve<ome::Color>{
                TestCyanParticles::color_from,
                TestCyanParticles::color_to},
            .scale = ome::InterpolationCurve<float>{
                TestCyanParticles::scale_from,
                TestCyanParticles::scale_to},
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint = blueprint_,
            .emission_rate_     = 50,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<Particles_> particles_ = std::make_shared<Particles_>();

  public:
    explicit TestSphereEmitter(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("SphereParticles").up();
    }
};

// -----------------------------------------------------------------------
// Ball — radius 1 m
// -----------------------------------------------------------------------
class TestBallEmitter : public ome::TransformNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {
            .initial_position = [] {
                return ome::math::sample_uniform(
                    ome::math::AnyRegion<3, float>(ome::math::Ball<3, float>{ {}, 0.7f }),
                    rng);
            },
            .initial_velocity = ome::Vec3f{},
            .angular_speed    = 0.0f,
            .time_to_live     = TestCyanParticles::time_to_live,
            .acceleration     = ome::Vec3f{},
            .color            = ome::InterpolationCurve<ome::Color>{
                TestCyanParticles::color_from,
                TestCyanParticles::color_to},
            .scale = ome::InterpolationCurve<float>{
                TestCyanParticles::scale_from,
                TestCyanParticles::scale_to},
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint = blueprint_,
            .emission_rate_     = 50,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<Particles_> particles_ = std::make_shared<Particles_>();

  public:
    explicit TestBallEmitter(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("BallParticles").up();
    }
};

// -----------------------------------------------------------------------
// Box — 2 × 2 × 2 m
// -----------------------------------------------------------------------
class TestBoxEmitter : public ome::TransformNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {
            .initial_position = [] {
                return ome::math::sample_uniform(
                    ome::math::AnyRegion<3, float>(
                        ome::math::Box<3, float>{ { -0.7f, -0.7f, -0.7f }, { 0.7f, 0.7f, 0.7f } }),
                    rng);
            },
            .initial_velocity = ome::Vec3f{},
            .angular_speed    = 0.0f,
            .time_to_live     = TestCyanParticles::time_to_live,
            .acceleration     = ome::Vec3f{},
            .color            = ome::InterpolationCurve<ome::Color>{
                TestCyanParticles::color_from,
                TestCyanParticles::color_to},
            .scale = ome::InterpolationCurve<float>{
                TestCyanParticles::scale_from,
                TestCyanParticles::scale_to},
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint = blueprint_,
            .emission_rate_     = 50,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<Particles_> particles_ = std::make_shared<Particles_>();

  public:
    explicit TestBoxEmitter(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("BoxParticles").up();
    }
};

// -----------------------------------------------------------------------
// Cone — apex at origin, pointing up, ≈34° half-angle, 1.4 m tall
// -----------------------------------------------------------------------
class TestConeEmitter : public ome::TransformNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {
            .initial_position = [] {
                return ome::math::sample_uniform(
                    ome::math::AnyRegion<3, float>(
                        ome::math::Cone<3, float>{ {},
                                                   { 0.0f, 1.0f, 0.0f },
                                                   0.6f, // ≈34° half-angle
                                                   1.4f }),
                    rng);
            },
            .initial_velocity = ome::Vec3f{},
            .angular_speed    = 0.0f,
            .time_to_live     = TestCyanParticles::time_to_live,
            .acceleration     = ome::Vec3f{},
            .color            = ome::InterpolationCurve<ome::Color>{
                TestCyanParticles::color_from,
                TestCyanParticles::color_to},
            .scale = ome::InterpolationCurve<float>{
                TestCyanParticles::scale_from,
                TestCyanParticles::scale_to},
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint = blueprint_,
            .emission_rate_     = 50,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<Particles_> particles_ = std::make_shared<Particles_>();

  public:
    explicit TestConeEmitter(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("ConeParticles").up();
    }
};

// -----------------------------------------------------------------------
// Burst — fast outward in all directions, decelerates, 3 m sphere
// -----------------------------------------------------------------------
class TestBurstEmitter : public ome::TransformNode
{
    class Particles_ : public ome::ParticleEmitterNode
    {
        struct TestBurstParticles
        {
        static inline const auto color_from    = ome::Color::rgba(1.0f, 0.9f, 0.5f, 1.0f);
        static inline const auto color_to      = ome::Color::rgba(1.0f, 0.4f, 0.0f, 0.0f);
        static constexpr float   scale_from   = 0.25f;
        static constexpr float   scale_to     = 0.0f;
        static constexpr float   time_to_live = 6.0f;
        };

        static inline const ome::ParticleScheme blueprint_ = {
            .initial_position = ome::Vec3f{},
            .initial_velocity = [] {
                return ome::math::sample_uniform(
                    ome::math::AnyRegion<3, float>(ome::math::Sphere<3, float>{ {}, 6.0f }),
                    rng);
            },
            .angular_speed = 0.0f,
            .time_to_live  = TestBurstParticles::time_to_live,
            .acceleration  = ome::Vec3f{ -6.0f },
            .color         = ome::InterpolationCurve<ome::Color>{
                TestBurstParticles::color_from,
                TestBurstParticles::color_to},
            .scale = ome::InterpolationCurve<float>{
                TestBurstParticles::scale_from,
                TestBurstParticles::scale_to},
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint = blueprint_,
            .emission_rate_     = 80,
        };

      public:
        Particles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

    std::shared_ptr<Particles_> particles_ = std::make_shared<Particles_>();

  public:
    explicit TestBurstEmitter(ome::Vec3f position)
        : TransformNode()
    {
        set_local_transform({ .position = position });
        extending(*this).add(particles_).named("BurstParticles").up();
    }
};

} // namespace soccernoid::spikes
