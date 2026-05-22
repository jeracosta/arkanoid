#pragma once

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>

#include <GL/gl.h>
#include <GL/glu.h>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/render_frame.hpp"
#include "oh-my-engine/open_gl/render_quad.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/projectile.hpp"

// Player mesh: assets/meshes/dragon/Dragon.fbx (geometry via Mesh::load).
// Optional PNG: assets/textures/dragon_texture.png or dragon.png
//
// Skeleton is baked to static pose via Assimp aiProcess_PreTransformVertices.
// Missing dragon mesh → blue sphere fallback

namespace {

struct DragonPlayerDrawSlot
{
    bool                       tried_load = false;
    std::shared_ptr<ome::Mesh> mesh{};
    ome::Material              material;
};

inline DragonPlayerDrawSlot &
dragon_player_slot()
{
    static DragonPlayerDrawSlot slot;
    return slot;
}

} // namespace

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
    static constexpr float player_radius_ = 0.2f;

    Configuration config_;
    bool          can_move_ = true;

    static constexpr float aim_sweep_speed_     = 2.0f;
    static constexpr float aim_sweep_amplitude_ = 0.7f;
    static constexpr float shoot_force_         = 5.0f;

    class AimArrowNode : public ome::Node
    {
      private:
        float         current_angle_ = 0.0f;
        ome::Material material_;

        static constexpr float shaft_length_ = 1.2f;
        static constexpr float shaft_width_  = 0.15f;
        static constexpr float head_length_  = 0.5f;
        static constexpr float head_width_   = 0.35f;
        static constexpr float arrow_offset_ = 0.8f;
        static constexpr float arrow_height_ = 0.05f;

        void
        render_arrow_()
        {
            auto *player     = static_cast<PlayerNode *>(parent());
            auto  player_pos = player->transform<ome::Space::World>().position;

            ome::Orientation rotation;
            rotation.rotate(current_angle_, ome::up);
            auto dir = rotation * ome::forward;

            ome::Vec3f perp = { dir[2], 0.0f, -dir[0] };

            auto base = player_pos + dir * arrow_offset_ + ome::up * arrow_height_;

            // Shaft (rectangle)
            auto shaft_start = base;
            auto shaft_end   = base + dir * shaft_length_;

            std::array<ome::Vec3f, 4> shaft = {
                shaft_start - perp * shaft_width_,
                shaft_start + perp * shaft_width_,
                shaft_end + perp * shaft_width_,
                shaft_end - perp * shaft_width_,
            };
            ome::open_gl::render_quad(shaft, material_);

            // Arrowhead (triangle)
            auto head_base = shaft_end;
            auto head_tip  = head_base + dir * head_length_;

            // Left half of triangle
            std::array<ome::Vec3f, 4> left_head = {
                head_base - perp * shaft_width_,
                head_base - perp * head_width_,
                head_tip,
                head_tip,
            };
            ome::open_gl::render_quad(left_head, material_);

            // Right half of triangle
            std::array<ome::Vec3f, 4> right_head = {
                head_base + perp * shaft_width_,
                head_base + perp * head_width_,
                head_tip,
                head_tip,
            };
            ome::open_gl::render_quad(right_head, material_);

            // Center of triangle (fills gap between left and right)
            std::array<ome::Vec3f, 4> center_head = {
                head_base - perp * shaft_width_,
                head_base + perp * shaft_width_,
                head_tip,
                head_tip,
            };
            ome::open_gl::render_quad(center_head, material_);
        }

        void
        on_shoot_()
        {
            auto *player = static_cast<PlayerNode *>(parent());

            auto &projectile = game()->root_node()->emplace_child<ProjectileNode>();

            auto player_pos = player->transform<ome::Space::World>().position;
            projectile.position(player_pos + ome::up * 0.5f);

            ome::Orientation rotation;
            rotation.rotate(current_angle_, ome::up);
            auto shoot_direction = rotation * ome::forward;
            auto velocity        = shoot_direction * shoot_force_ + ome::up * 0.2f;

            projectile.update_kinematic<ome::Space::World>(
                [&](auto &k) { k.velocity = velocity; });

            player->enable_movement();
            schedule_unmount();
        }

      public:
        AimArrowNode()
        {
            material_.color.ambient  = ome::Color::white();
            material_.color.diffuse  = ome::Color::white();
            material_.color.specular = ome::Color::hex(0x000000FF);
            material_.color.emission = ome::Color::hex(0x000000FF);
            material_.blend_mode     = ome::BlendMode::alpha();
        }

        void
        on_mount_() override
        {
            auto *player      = static_cast<PlayerNode *>(parent());
            player->can_move_ = false;

            hold(game()->input.bind(Action::PlayerShoot, &AimArrowNode::on_shoot_, this));
        }

        void
        on_tick_() override
        {
            current_angle_ = std::sin(game()->time.elapsed() * aim_sweep_speed_) * aim_sweep_amplitude_;
            render_arrow_();
        }
    };

    void
    ensure_dragon_mesh_loaded_()
    {
        auto &dr = dragon_player_slot();

        if (dr.tried_load)
        {
            return;
        }

        dr.tried_load       = true;
        const auto mesh_path = FilesystemPaths::meshes / "dragon" / "Dragon.fbx";

        std::filesystem::path texture_png;
        for (auto candidate : {
                 FilesystemPaths::textures / "dragon_texture.png",
                 FilesystemPaths::textures / "dragon.png",
             })
        {
            if (std::filesystem::exists(candidate))
            {
                texture_png = std::move(candidate);
                break;
            }
        }

        if (!std::filesystem::exists(mesh_path))
        {
            log(std::format(
                "Dragon mesh missing (expected `{}`). Using blue sphere fallback.", mesh_path.string()));

            return;
        }

        try
        {
            dr.mesh = ome::Mesh::load(mesh_path);
            dr.mesh->recenter();

            {
                constexpr float target_extent = 2.5f;
                auto            sz            = dr.mesh->size();
                float           max_dim       = std::max({ sz[0], sz[1], sz[2] });
                dr.mesh->resize(sz * (target_extent / max_dim));
            }

            std::shared_ptr<ome::Texture> color_tex;
            if (!texture_png.empty())
            {
                color_tex = ome::Texture::load(texture_png);
            }

            if (color_tex)
            {
                color_tex->set_wrap({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });
                color_tex->set_filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                dr.material.texture = std::move(color_tex);
            }
        }
        catch (const std::exception &e)
        {
            dr.mesh.reset();
            dr.material = {};

            log(std::format(
                "Failed loading dragon `{}`: {} — using blue sphere.", mesh_path.string(), e.what()));
        }
    }

    void
    on_render_(ome::RenderFrame &frame) override
    {
        ensure_dragon_mesh_loaded_();

        auto &dr = dragon_player_slot();

        if (dr.mesh)
        {
            auto draw_tr = transform<ome::Space::World>();

            ome::Orientation face_align = ome::Orientation::identity();
            face_align.rotate(ome::pi, ome::up);
            draw_tr.orientation = draw_tr.orientation * face_align;

            frame.draw_commands.push_back(ome::DrawCommand{
                .mesh      = dr.mesh,
                .materials = { dr.material },
                .transform = draw_tr,
            });

            return;
        }

        auto                      position = transform<ome::Space::World>().position;
        static const ome::Color   sphere_color = ome::Color::hex(0x1486cc);

        glColor(sphere_color);
        glPushMatrix();
        {
            GLUquadric *q = gluNewQuadric();
            gluQuadricNormals(q, GLU_SMOOTH);
            glTranslatef(position[0], position[1], position[2]);
            gluSphere(q, player_radius_, 32, 32);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }

    void
    process_movement_()
    {
        if (!can_move_)
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

        update_kinematic<ome::Space::World>([&](auto &k)
        {
            k.velocity += direction * config_.movement_force * game()->time.delta();

            if (norm(k.velocity) > config_.max_speed)
            {
                k.velocity = normalized(k.velocity) * config_.max_speed;
            }
        });
    }

    void
    clamp_to_bounds_()
    {
        update_transform<ome::Space::Local>([](auto &t)
        {
            const float min = -map_half_extent + player_radius_;
            const float max =  map_half_extent - player_radius_;
            t.position[0] = std::clamp(t.position[0], min, max);
            t.position[2] = std::clamp(t.position[2], min, max);
        });
    }

  public:
    PlayerNode(const Configuration &config)
        : config_(config)
    {
        emplace_child<AimArrowNode>().rename("AimArrow");
    }

    void
    enable_movement()
    {
        can_move_ = true;
    }

    void
    start_aiming()
    {
        if (!can_move_)
        {
            return;
        }

        emplace_child<AimArrowNode>().rename("AimArrow");
    }

    void
    on_tick_() override
    {
        process_movement_();
        ome::KinematicNode::on_tick_();
        clamp_to_bounds_();
    }

    void
    on_mount_() override
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = ome::up * 1.5f; });
        log("Player mounted");
    }
};

} // namespace soccernoid
