#pragma once

#include <memory>

#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/nodes/fire_explosion.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

class ExplosiveBarrelNode : public ome::TransformNode
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

            projectile->update_kinematic([](auto &k) { k.velocity *= 2.0f; });

            auto world_position = hitbox<ome::Space::World>().center();

            auto &explosion = game()->root_node()->emplace_child<FireExplosionNode>();
            explosion.position(world_position);

            auto *barrel = static_cast<ExplosiveBarrelNode *>(parent());
            barrel->request_unmount();
        }

      public:
        HitboxNode(const ome::Vec3f &size, const ome::Vec3f &center)
            : ome::HitboxNode(size, center)
        {
        }
    };

  private:
    ome::MeshNode *mesh_;

  public:
    ExplosiveBarrelNode()
    {
        auto barrel_mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.barrel);
        barrel_mesh->resize({ 0.6f, 0.8f, 0.6f });

        auto material = ome::Material{
            .texture = textures.barrel,
        };

        auto size   = barrel_mesh->size();
        auto center = barrel_mesh->center();

        // offset so the barrel bottom sits at the node origin
        auto offset = ome::Vec3f{ 0, size[1] / 2.0f - center[1], 0 };

        mesh_ = &emplace_child<ome::MeshNode>(barrel_mesh, material);
        mesh_->position(offset).rename("BarrelMesh");

        emplace_child<HitboxNode>(size, center + offset).rename("BarrelHitbox");
    }
};

} // namespace soccernoid
