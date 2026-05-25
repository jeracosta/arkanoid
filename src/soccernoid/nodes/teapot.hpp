#pragma once

#include <cmath>
#include <memory>

#include "oh-my-engine/harmonic_oscillator_curve.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/nodes/progress_bar.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

class TeapotNode : public SoccernoidNode<ome::TransformNode>
{
  public:
    class HitboxNode : public ome::HitboxNode
    {
      public:
        using ome::HitboxNode::HitboxNode;

        TeapotNode *
        teapot()
        {
            return static_cast<TeapotNode *>(parent());
        }

      protected:
        void
        on_collision_(ome::HitboxNode &other) override
        {
            if (!dynamic_cast<ProjectileNode *>(other.parent()))
            {
                return;
            }

            auto *teapot = this->teapot();

            if (!teapot->is_invulnerable_())
            {
                auto prev_life = teapot->life_;
                teapot->life_  = std::max(0.0f, teapot->life_ - 0.2f);

                auto damage = prev_life - teapot->life_;
                teapot->game()->events.emit(
                    ScoreAwarded{ static_cast<int>(std::round(damage * 100.0f)) });

                teapot->lifebar_->set_progress(teapot->life_);

                teapot->size_process_->restart();
                teapot->color_process_->restart();
            }
        }
    };

  private:
    std::unique_ptr<ome::CurveProcess<float>> size_process_;
    std::unique_ptr<ome::CurveProcess<float>> color_process_;

    ome::MeshNode *mesh_node_ = nullptr;
    ome::Vec3f     mesh_size_ = {};

    float            life_    = 0.9f;
    bool             won_     = false;
    ProgressBarNode *lifebar_ = nullptr;

    bool
    is_invulnerable_() const
    {
        return !size_process_->is_completed();
    }

  public:
    TeapotNode()
    {
        auto mesh  = static_cast<std::shared_ptr<ome::Mesh>>(meshes.teapot);
        mesh_size_ = mesh->size();

        auto spring   = std::make_shared<ome::HarmonicOscillatorCurve>();
        size_process_ = std::make_unique<ome::CurveProcess<float>>(spring, 1.5f);

        auto color_curve = std::make_shared<ome::Interpolation<float>>(0.02f, 0.0f);
        color_process_   = std::make_unique<ome::CurveProcess<float>>(color_curve, 0.5f);

        auto &node = emplace_child<ome::MeshNode>(mesh, ome::Material{ .shininess = 25.0f });
        mesh_node_ = &node;

        // Hitbox hangs off the teapot (static), not the mesh node, whose per-frame spin and
        // pulse scale would otherwise make the hitbox's world AABB constantly change size.
        emplace_child<HitboxNode>(mesh_size_, mesh->center());

        auto &lifebar = emplace_child<ProgressBarNode>(ProgressBarNode::Configuration{
            .materials = {
                .foreground = {
                    .color = {
                        .diffuse  = ome::Color::transparent(),
                        .specular = ome::Color::transparent(),
                        .emission = ome::Color::red(),
                    }
                },
                .background = {
                    .color = {
                        .ambient = ome::Color::rgb(0.3f, 0.3f, 0.3f),
                        .diffuse  = ome::Color::transparent(),
                        .specular = ome::Color::transparent(),
                    }
                },
                .ghosting = {
                    .color = {
                        .diffuse  = ome::Color::transparent(),
                        .specular = ome::Color::transparent(),
                        .emission = ome::Color::white(),
                    }
                },
            },
            .size = { 2.0f, 0.2f},
            .slide_duration = 1.0f,
            .ghost_delay = 0.5f,
        });
        lifebar.rename("Lifebar");
        lifebar.position({ 0.0f, mesh_size_[1] / 2.0f + 3.5f, 0.0f });
        lifebar_ = &lifebar;
    }

    void
    on_mount_() override
    {
        size_process_->complete();
        color_process_->complete();
    }

    void
    on_tick_() override
    {
        size_process_->update(game()->time.delta());
        color_process_->update(game()->time.delta());

        auto angle = game()->time.elapsed();

        ome::Orientation rot;
        rot.rotate(angle, ome::up);

        auto vertical = std::sin(angle);

        auto size = size_process_->value();

        mesh_node_->update_transform<ome::Space::Local>([&](auto &t)
        {
            t.orientation = rot;
            t.position    = { 0.0f, vertical, 0.0f };
            t.scale       = { size, size, size };
        });

        auto color_value = color_process_->value();

        auto rainbow = ome::Color::hsv(angle * 60.0f, 0.9f, 0.4f);

        mesh_node_->update_material([&](auto &mat)
        {
            mat.color.ambient  = rainbow;
            mat.color.diffuse  = rainbow;
            mat.color.emission = rainbow + ome::Color::white() * color_value;
        });

        lifebar_->position({ 0.0f, vertical + mesh_size_[1] / 2.0f + 3.5f, 0.0f });

        if (life_ <= 0.0f && lifebar_->is_progress_completed() && !won_)
        {
            won_ = true;
            game()->events.emit(PlayerVictorious{});
        }
    }
};

} // namespace soccernoid
