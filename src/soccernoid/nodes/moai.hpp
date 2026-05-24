#pragma once

#include <memory>

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/sphere.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/soccernoid.hpp"

namespace soccernoid {

class MoaiNode : public ome::TransformNode
{
    class DeintegrationParticles_ : public ome::ParticleEmitterNode
    {
        static inline const ome::ParticleScheme blueprint_ = {

            .initial_position = { ome::Box::from_size(ome::Vec3f{ 0.8f, 1.0f, 0.8f }), rng },

            .initial_velocity = { ome::math::Sphere<3>({}, 0.1f), rng },

            .time_to_live = 5.0f,

            .color =
                [](const auto &ctx)
        {
            const auto &particle = ctx.particle;
            float       t        = particle.progress();

            // deterministic pseudo-random seed from position
            float seed = std::abs(dot(particle.position, { 12.9898f, 78.233f, 45.164f }));
            seed -= std::floor(seed);

            // random starting color between white and green
            auto rgb = ome::Vec3f(1.0f - seed, 1.0f, 1.0f - seed);

            return ome::Color::rgb(rgb * (1.0f - t));
        },

            .scale = ome::Interpolation{ 0.15f, 0.0f },

            .blend_mode = ome::BlendMode::additive(),
        };

        static inline const ome::ParticleEmitterNode::Settings config_ = {
            .particle_blueprint    = blueprint_,
            .trigger_rate          = 1000,
            .particles_per_trigger = 300,
            .total_emissions_count = 1,
        };

      public:
        DeintegrationParticles_()
            : ome::ParticleEmitterNode(config_)
        {
        }
    };

  public:
    class HitboxNode : public ome::HitboxNode
    {
        void
        on_collision_(ome::HitboxNode &other) override
        {
            auto *projectile = dynamic_cast<ProjectileNode *>(other.parent());
            if (!projectile)
            {
                return;
            }

            auto world_pos = hitbox<ome::Space::World>().center();

            auto &particles = game()->root_node()->emplace_child<DeintegrationParticles_>();
            particles.position(world_pos);

            auto *moai = static_cast<MoaiNode *>(parent());
            static_cast<Soccernoid *>(game())->events.emit(ScoreAwarded{ 5 });
            moai->request_unmount();
        }

      public:
        HitboxNode(const ome::Vec3f &size, const ome::Vec3f &center)
            : ome::HitboxNode(size, center)
        {
        }
    };

  private:
    ome::MeshNode *mesh_node_ = nullptr;

  public:
    MoaiNode()
    {
        auto mesh     = static_cast<std::shared_ptr<ome::Mesh>>(meshes.moai);
        auto material = ome::Material{ .texture = textures.moai };

        mesh->resize({ 0.8f, 1.0f, 0.8f });

        auto size   = mesh->size();
        auto center = mesh->center();

        // offset so the bottom sits at the node origin
        auto offset = ome::Vec3f{ 0.0f, size[1] / 2.0f - center[1], 0.0f };

        mesh_node_ = &emplace_child<ome::MeshNode>(mesh, material);
        mesh_node_->position(offset).rename("MoaiMesh");

        emplace_child<HitboxNode>(size, center + offset).rename("MoaiHitbox");
    }
};

} // namespace soccernoid
