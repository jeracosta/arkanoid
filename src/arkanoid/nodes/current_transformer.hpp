#pragma once

#include <memory>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "arkanoid/constants.hpp"
#include "arkanoid/nodes/energy_explosion.hpp"
#include "arkanoid/nodes/projectile.hpp"

namespace arkanoid {

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

            auto  world_pos = transformer->transform<ome::Space::World>().position;
            auto  velocity  = projectile->kinematic<ome::Space::World>().velocity;
            float speed     = ome::math::norm(velocity) * 1.05f;

            game()->root_node()->emplace_child<EnergyExplosionNode>().position(world_pos);

            constexpr float spread = ome::pi / 4.0f;

            for (float sign : { -1.0f, 1.0f })
            {
                auto direction = ome::Orientation().steer_yaw(sign * spread) * velocity;

                game()->schedule([transformer, world_pos, direction, speed]
                {
                    auto *level = transformer->parent();
                    if (!level)
                    {
                        return;
                    }

                    auto &spawned = level->emplace_child<ProjectileNode>();
                    spawned.position(world_pos + ome::up * 0.5f);
                    spawned.velocity(ome::math::normalized(direction) * speed);
                });
            }

            transformer->request_unmount();
            projectile->request_unmount();
        }

      public:
        HitboxNode(const ome::Vec3f &size, const ome::Vec3f &center)
            : ome::HitboxNode(size, center)
        {
        }
    };

    CurrentTransformerNode()
    {
        auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.transformer);
        mesh->resize({ 0.8f, 0.8f, 0.8f });

        auto material = ome::Material{ .texture = textures.transformer };

        auto size   = mesh->size();
        auto center = mesh->center();

        // offset so the bottom sits at the node origin
        auto offset = ome::Vec3f{ 0.0f, size[1] / 2.0f - center[1], 0.0f };

        emplace_child<ome::MeshNode>(mesh, material).position(offset).rename("TransformerMesh");
        emplace_child<HitboxNode>(size, center + offset).rename("TransformerHitbox");
    }
};

} // namespace arkanoid
