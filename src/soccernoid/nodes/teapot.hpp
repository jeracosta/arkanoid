#pragma once

#include <cmath>
#include <memory>

#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/spline.hpp"
#include "oh-my-engine/spring_curve.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

class TeapotNode : public ome::TransformNode
{
    class HitboxNode_ : public ome::HitboxNode
    {
      public:
        using ome::HitboxNode::HitboxNode;

        TeapotNode *
        teapot()
        {
            return static_cast<TeapotNode *>(parent()->parent());
        }

      protected:
        void
        on_collision_(ome::HitboxNode &other) override
        {
            if (dynamic_cast<ProjectileNode *>(other.parent()))
            {
                teapot()->process_->restart();
            }
        }
    };

    std::unique_ptr<ome::CurveProcess<float>> process_;
    ome::MeshNode                            *mesh_node_ = nullptr;
    ome::Vec3f                                mesh_size_ = {};

  public:
    TeapotNode()
    {
        auto mesh  = static_cast<std::shared_ptr<ome::Mesh>>(meshes.teapot);
        mesh_size_ = mesh->size();

        auto envelope = std::make_shared<ome::Spline<float>>(ome::Spline<float>::catmull_rom({
            { 0.0f, 0.0f },
            { 0.5f, 1.0f },
            { 1.0f, 0.0f },
        }));

        auto spring         = std::make_shared<ome::SpringCurve>();
        spring->envelope_ = std::move(envelope);
        process_          = std::make_unique<ome::CurveProcess<float>>(spring, 1.5f);

        auto &node = emplace_child<ome::MeshNode>(mesh, ome::Material{ .shininess = 25.0f });
        node.emplace_child<HitboxNode_>(mesh_size_, mesh->center());

        mesh_node_ = &node;
    }

    void
    on_mount_() override
    {
        process_->complete();
    }

    void
    on_tick_() override
    {
        process_->update(game()->time.delta());

        auto angle = game()->time.elapsed();
        auto s     = process_->value();

        ome::Orientation rot;
        rot.rotate(angle, ome::up);

        auto vertical = std::sin(angle);

        mesh_node_->update_transform<ome::Space::Local>([&](auto &t)
        {
            t.orientation = rot;
            t.position    = { 0.0f, vertical, 0.0f };
            t.scale       = { s, s, s };
        });

        mesh_node_->update_material([&](auto &mat)
        {
            mat.color.ambient = ome::Color::hsv(angle * 60.0f, 0.9f, 1.0f);
            mat.color.diffuse = mat.color.ambient;
        });
    }
};

} // namespace soccernoid
