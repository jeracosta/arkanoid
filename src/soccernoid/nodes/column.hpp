#pragma once

#include <memory>

#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/nodes/fire.hpp"

namespace soccernoid {

class ColumnNode : public ome::TransformNode
{
    ome::MeshNode        *mesh_;
    soccernoid::FireNode *fire_;
    ome::HitboxNode      *hitbox_;

  public:
    ColumnNode()
    {
        auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.column);
        mesh->resize({ 0.8f, 3.0f, 0.8f });

        auto size   = mesh->size();
        auto center = mesh->center();

        auto material = ome::Material{
            .texture = textures.column,
        };

        mesh_ = &emplace_child<ome::MeshNode>(mesh, material);
        mesh_->rename("ColumnMesh");

        hitbox_ = &emplace_child<ome::HitboxNode>(size, center);
        hitbox_->rename("ColumnHitbox");

        fire_ = &emplace_child<soccernoid::FireNode>();
        fire_->position({ 0.0f, center[1] + size[1] * 0.5f, 0.0f }).rename("Fire");
    }
};

} // namespace soccernoid
