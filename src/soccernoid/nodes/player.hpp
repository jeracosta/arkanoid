#pragma once

#include <exception>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>

#include <GL/gl.h>
#include <GL/glu.h>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/open_gl/render_mesh.hpp"
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
    Configuration config_;

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
        constexpr float         radius = 0.2f;
        static const ome::Color color  = ome::Color::hex(0x1486cc);

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
            gluSphere(q, radius, 32, 32);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }

    void
    process_movement_()
    {
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
    shoot_()
    {
        auto &projectile = game()->root_node()->emplace_child<ProjectileNode>();

        projectile.position(transform<ome::Space::World>().position);
        projectile.velocity(0.2 * ome::up + 5.0 * ome::forward);
    }

  public:
    PlayerNode(const Configuration &config)
        : config_(config)
    {
    }

    void
    on_tick_() override
    {
        process_movement_();
        ome::KinematicNode::on_tick_();
        render_();
    }

    void
    on_mount_() override
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = ome::up * 1.5f; });
        log("Hola");

        hold(game()->input.bind(Action::PlayerShoot, &PlayerNode::shoot_, this));
    }
};

} // namespace soccernoid
