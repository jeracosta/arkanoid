#pragma once

#include <format>
#include <memory>
#include <numbers>
#include <random>

#include "oh-my-engine/harmonic_oscillator_curve.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/spline.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/nodes/energy_explosion.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

class CurrentTransformerNode : public ome::TransformNode
{
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

            auto *transformer = static_cast<CurrentTransformerNode *>(parent());
            std::uniform_int_distribution<int> coin(0, 3);

            if (coin(transformer->rng_) != 0)
            {
                transformer->size_process_->restart();
                transformer->color_process_->restart();
            }
            else
            {
                auto world_pos = transformer->transform<ome::Space::World>().position;

                auto &explosion = game()->root_node()->emplace_child<EnergyExplosionNode>();
                explosion.position(world_pos);

                auto  velocity = projectile->kinematic<ome::Space::World>().velocity;
                float speed    = ome::math::norm(velocity) * 2.5f;

                for (int i = 0; i < 2; ++i)
                {
                    float angle = static_cast<float>(i) * std::numbers::pi_v<float>
                                  - std::numbers::pi_v<float> / 2.0f;
                    auto direction = ome::Vec3f{ std::cos(angle), 0.2f, std::sin(angle) };

                    game()->schedule([transformer, world_pos, direction, speed]
                    {
                        if (auto *level = transformer->parent())
                        {
                            auto &proj = level->emplace_child<ProjectileNode>();
                            proj.rename(std::format("TransformerProj_{}",
                                                    reinterpret_cast<uintptr_t>(&proj)));
                            proj.update_transform<ome::Space::Local>([&](auto &t)
                            { t.position = world_pos + ome::up * 0.5f; });
                            proj.velocity(ome::math::normalized(direction) * speed);
                        }
                    });
                }

                transformer->request_unmount();
            }
        }

      public:
        HitboxNode(const ome::Vec3f &size, const ome::Vec3f &center)
            : ome::HitboxNode(size, center)
        {
        }
    };

  private:
    ome::MeshNode                                 *mesh_node_ = nullptr;
    std::unique_ptr<ome::CurveProcess<float>>      size_process_;
    std::unique_ptr<ome::CurveProcess<ome::Color>> color_process_;
    static inline std::mt19937                     rng_{ std::random_device{}() };
    friend class HitboxNode;

  public:
    CurrentTransformerNode()
    {
        auto mesh     = static_cast<std::shared_ptr<ome::Mesh>>(meshes.transformer);
        auto material = ome::Material{ .texture = textures.transformer };
        mesh->resize({ 0.8f, 0.8f, 0.8f });

        auto size   = mesh->size();
        auto center = mesh->center();

        // offset so the bottom sits at the node origin
        auto offset = ome::Vec3f{ 0.0f, size[1] / 2.0f - center[1], 0.0f };

        mesh_node_ = &static_cast<ome::MeshNode &>(emplace_child<ome::MeshNode>(mesh, material)
                                                       .position(offset)
                                                       .rename("TransformerMesh"));

        emplace_child<HitboxNode>(size, center + offset).rename("TransformerHitbox");

        auto wiggle   = std::make_shared<ome::HarmonicOscillatorCurve>();
        size_process_ = std::make_unique<ome::CurveProcess<float>>(wiggle, 3.0f);

        auto flash = std::make_shared<ome::Interpolation<ome::Color>>(
            ome::Color::rgb(0.6f, 1.0f, 0.7f), ome::Color::black());
        color_process_ = std::make_unique<ome::CurveProcess<ome::Color>>(flash, 6.0f);

        size_process_->complete();
        color_process_->complete();
    }

    void
    on_tick_() override
    {
        size_process_->update(game()->time.delta());
        color_process_->update(game()->time.delta());

        auto size = size_process_->value();

        mesh_node_->update_transform<ome::Space::Local>([&](auto &t)
        { t.scale = { size, size, size }; });

        mesh_node_->update_material([&](auto &mat)
        { mat.color.emission = color_process_->value(); });
    }
};

} // namespace soccernoid
