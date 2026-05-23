#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
#include <algorithm>
#include <cmath>
#include <memory>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/render_frame.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

class PlayerNode : public ome::KinematicNode
{
  public:
    struct Configuration
    {
        float movement_force;
        float max_speed;
        float speed_decay;

        static Configuration
        make_harry()
        {
            return {
                .movement_force = 10.0f,
                .max_speed      = 5.0f,
                .speed_decay    = 2.0f,
            };
        }
    };

  private:
    static std::shared_ptr<ome::Mesh>
    mesh_()
    {
        auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.dragon);
        mesh->recenter();
        constexpr float target_extent = 2.5f;
        auto            sz            = mesh->size();
        float           max_dim       = std::max({ sz[0], sz[1], sz[2] });
        mesh->resize(sz * (target_extent / max_dim));
        return mesh;
    }

    ome::Material
    material_()
    {
        // TODO: Configure player material
        return {};
    }

    static constexpr float player_radius_ = 0.2f;

    Configuration config_;

    bool aiming_;

    static constexpr float aim_sweep_speed_     = 2.0f;
    static constexpr float aim_sweep_amplitude_ = 0.7f;
    static constexpr float shoot_force_         = 5.0f;

    class AimArrowNode_ : public ome::Node
    {
      private:
        float               current_angle_ = 0.0f;
        ome::MeshNode      *arrow_mesh_;

        void
        on_shoot_()
        {
            auto *player = static_cast<PlayerNode *>(parent());

            auto rotation        = ome::Orientation{}.rotate(current_angle_, ome::up);
            auto shoot_direction = rotation * ome::forward;

            player->shoot(shoot_direction);
            player->aiming_ = false;

            request_unmount();
        }

      public:
        AimArrowNode_()
        {
            auto mesh     = static_cast<std::shared_ptr<ome::Mesh>>(meshes.arrow);
            mesh->resize({ 0.5f, 0.5f, 0.5f });
            auto material = ome::Material{};
            material.color.emission = ome::Color::rgb(0.0f, 1.0f, 0.0f);
            arrow_mesh_ = &emplace_child<ome::MeshNode>(mesh, material);
            arrow_mesh_->rename("ArrowMesh");
        }

        void
        on_mount_() override
        {
            auto *player    = static_cast<PlayerNode *>(parent());
            player->aiming_ = true;

            hold(game()->input.bind(Action::PlayerShoot, &AimArrowNode_::on_shoot_, this));
        }

        void
        on_tick_() override
        {
            current_angle_
                = std::sin(game()->time.elapsed() * aim_sweep_speed_) * aim_sweep_amplitude_;

            static constexpr float arrow_offset_ = 1.5f;
            static constexpr float arrow_height_ = 0.05f;

            auto rotation = ome::Orientation{}.rotate(current_angle_, ome::up);
            auto dir      = rotation * ome::forward;

            arrow_mesh_->position(dir * arrow_offset_ + ome::up * arrow_height_);
            arrow_mesh_->orientation(rotation);
        }
    };

    void
    on_render_(ome::RenderFrame &frame) override
    {
        auto transform = this->transform<ome::Space::World>();

        ome::Orientation face_align = ome::Orientation::identity();
        face_align.rotate(ome::pi, ome::up);
        transform.orientation = transform.orientation * face_align;

        frame.draw_commands.push_back(ome::DrawCommand{
            .mesh      = mesh_(),
            .materials = { material_() },
            .transform = transform,
        });
    }

    void
    process_movement_()
    {
        if (aiming_)
        {
            return;
        }

        velocity(kinematic().velocity * std::exp(-config_.speed_decay * game()->time.delta()));

        struct MoveSpecification
        {
            Action     action;
            ome::Vec3f direction;

            operator const ome::Vec3f &() const
            {
                return direction;
            }
        };

        static auto moves = std::to_array<MoveSpecification>({
            { Action::PlayerLeft, ome::left },
            { Action::PlayerRight, ome::right },
        });

        auto is_active = [this](const MoveSpecification &move)
        { return game()->input.is_pressed(move.action); };

        auto active_moves = moves | std::views::filter(is_active);

        auto raw_direction = std::ranges::fold_left(active_moves, ome::Vec3f{}, std::plus{});

        if (norm(raw_direction) == 0)
        {
            return;
        }

        auto direction = normalized(raw_direction);

        update_kinematic<ome::Space::World>([&](auto &kinematic)
        {
            kinematic.velocity += direction * config_.movement_force * game()->time.delta();

            if (norm(kinematic.velocity) > config_.max_speed)
            {
                kinematic.velocity = normalized(kinematic.velocity) * config_.max_speed;
            }
        });
    }

  public:
    PlayerNode(const Configuration &config)
        : config_(config)
    {
        emplace_child<ome::HitboxNode>(mesh_()->size()).rename("Hitbox");
        emplace_child<AimArrowNode_>().rename("AimArrow");
    }

    void
    start_aiming()
    {
        if (aiming_)
        {
            return;
        }

        emplace_child<AimArrowNode_>().rename("AimArrow");
    }

    void
    shoot(ome::Vec3f direction)
    {
        auto position = transform<ome::Space::World>().position;

        auto &projectile = game()->root_node()->emplace_child<ProjectileNode>();
        projectile.position(position + normalized(direction) * 1.5f);

        auto velocity = normalized(direction) * shoot_force_ + ome::up * 0.2f;

        projectile.update_kinematic<ome::Space::World>([&](auto &k) { k.velocity = velocity; });
    }

    void
    on_tick_() override
    {
        process_movement_();
        ome::KinematicNode::on_tick_();
    }
};

} // namespace soccernoid


