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
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/open_gl/render_mesh.hpp"
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
    std::optional<ome::Sprite> sprite{};
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
    static constexpr float hitbox_size_   = 1.2f;

    Configuration config_;
    bool          can_move_ = true;

    static constexpr float aim_sweep_speed_     = 2.0f;
    static constexpr float aim_sweep_amplitude_ = 0.7f;
    static constexpr float shoot_force_         = 5.0f;

    class PlayerHitboxNode : public ome::HitboxNode
    {
      public:
        PlayerHitboxNode()
            : ome::HitboxNode(ome::Vec3f{ hitbox_size_, hitbox_size_, hitbox_size_ })
        {
        }
    };

    class AimArrowNode : public ome::Node
    {
      private:
        float         current_angle_  = 0.0f;
        ome::Vec3f    current_dir_    = ome::forward;
        ome::Material material_;

        static constexpr float shaft_length_ = 1.2f;
        static constexpr float shaft_width_  = 0.15f;
        static constexpr float head_length_  = 0.5f;
        static constexpr float head_width_   = 0.35f;
        static constexpr float arrow_offset_ = 0.8f;
        static constexpr float arrow_height_ = 0.05f;

        void
        update_direction_()
        {
            current_angle_ = std::sin(game()->time.elapsed() * aim_sweep_speed_) * aim_sweep_amplitude_;

            ome::Orientation rotation;
            rotation.rotate(current_angle_, ome::up);
            current_dir_ = rotation * ome::forward;
        }

        void
        render_arrow_()
        {
            auto *player     = static_cast<PlayerNode *>(parent());
            auto  player_pos = player->transform<ome::Space::World>().position;

            auto dir = current_dir_;

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
            auto *player     = static_cast<PlayerNode *>(parent());
            auto  player_pos = player->transform<ome::Space::World>().position;

            // Spawn projectile ahead of player, outside the hitbox
            auto spawn_pos = player_pos + current_dir_ * 1.0f + ome::up * 0.5f;
            auto velocity  = current_dir_ * shoot_force_ + ome::up * 0.2f;

            auto *level = player->parent();
            auto &projectile = level->emplace_child<ProjectileNode>();
            projectile.rename("PlayerProjectile");
            projectile.update_transform<ome::Space::Local>([&](auto &t)
            { t.position = spawn_pos; });
            projectile.velocity(velocity);

            player->enable_movement();
            schedule_unmount();
        }

      public:
        AimArrowNode()
        {
            material_.color      = ome::Color::white();
            material_.blend_mode = ome::BlendMode::alpha();
        }

        void
        on_mount_() override
        {
            hold(game()->input.bind(Action::PlayerShoot, &AimArrowNode::on_shoot_, this));
        }

        void
        on_tick_() override
        {
            update_direction_();
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
            dr.mesh->recenter_to_origin();
            dr.mesh->normalize_to_max_extent(2.5f);

            std::shared_ptr<ome::Texture> color_tex;
            if (!texture_png.empty())
            {
                color_tex = ome::Texture::load(texture_png);
            }

            if (dr.mesh->has_uv() && color_tex)
            {
                color_tex->set_wrap({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });
                color_tex->set_filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                dr.sprite.emplace(ome::Sprite{ .texture = std::move(color_tex) });
            }
        }
        catch (const std::exception &e)
        {
            dr.mesh.reset();
            dr.sprite.reset();

            log(std::format(
                "Failed loading dragon `{}`: {} — using blue sphere.", mesh_path.string(), e.what()));
        }
    }

    void
    render_()
    {
        static const ome::Color color = ome::Color::hex(0x1486cc);

        ensure_dragon_mesh_loaded_();

        auto &dr = dragon_player_slot();

        if (dr.mesh)
        {
            auto draw_tr = transform<ome::Space::World>();

            ome::Orientation face_align = ome::Orientation::identity();
            face_align.rotate(ome::pi, ome::up);
            draw_tr.orientation = draw_tr.orientation * face_align;

            ome::open_gl::MeshRenderTask{
                .mesh             = *dr.mesh,
                .transform        = draw_tr,
                .sprite           = dr.sprite,
                .modulate         = std::nullopt,
                .texture_env_mode = GL_MODULATE,
            }();

            return;
        }

        auto position = transform<ome::Space::World>().position;

        glColor(color);
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
        emplace_child<PlayerHitboxNode>().rename("Hitbox");
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
        render_();
    }

    void
    on_mount_() override
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = ome::up * 1.5f; });
        log("Player mounted");
    }
};

} // namespace soccernoid
