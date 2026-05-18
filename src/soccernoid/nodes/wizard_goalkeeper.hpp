#pragma once

#include <exception>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>

#include <GL/gl.h>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/open_gl/render_mesh.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace {

struct WizardGkDrawSlot
{
    bool                       tried_load = false;
    std::shared_ptr<ome::Mesh> mesh{};
    std::optional<ome::Sprite> sprite{};
};

inline WizardGkDrawSlot &
wizard_gk_draw_slot()
{
    static WizardGkDrawSlot slot;
    return slot;
}

} // namespace

namespace soccernoid {

// PolyArt wizard; hitbox matches HumanNode at the same logical position `{0,0,-5}`.
// Mesh pivot is its AABB centre (different from procedural ellipsoid). Tune `normalize`
// only if silhouette clips past the plateau edges.

class WizardGoalkeeperNode : public ome::HitboxNode
{
  public:
    struct Configuration
    {
        ome::Vec3f position = { 0.0f, 0.0f, 0.0f };
    };

  private:
    // Match HumanNode hitbox extents (same collision as before swapping to mesh).
    static constexpr float body_radius_ = 0.50f;
    static constexpr float body_height_ = 2.92f;
    static constexpr float head_radius_ = 0.24f;
    static constexpr ome::Vec3f hit_dims_{
        2.0f * body_radius_,
        body_height_ + 2.0f * head_radius_,
        2.0f * body_radius_,
    };
    static constexpr ome::Vec3f hit_offset_{ 0.0f, head_radius_, 0.0f };

    void
    ensure_wizard_mesh_loaded_()
    {
        auto &gk = wizard_gk_draw_slot();

        if (gk.tried_load)
        {
            return;
        }

        gk.tried_load       = true;
        const auto mesh_fbx = FilesystemPaths::meshes / "wizard" / "PolyArtWizardMesh.fbx";

        std::filesystem::path texture_path;
        for (auto candidate : {
                 FilesystemPaths::textures / "polyArtTex.png",
                 FilesystemPaths::textures / "poly_art_wizard.png",
             })
        {
            if (std::filesystem::exists(candidate))
            {
                texture_path = std::move(candidate);
                break;
            }
        }

        if (!std::filesystem::exists(mesh_fbx))
        {
            log(std::format(
                "Wizard mesh missing (`{}`). No mesh drawn.", mesh_fbx.string()));
            return;
        }

        try
        {
            gk.mesh = ome::Mesh::load(mesh_fbx);
            gk.mesh->recenter_to_origin();

            constexpr float goalie_scale = 3.0f;
            gk.mesh->normalize_to_max_extent(goalie_scale);

            if (gk.mesh->has_uv() && !texture_path.empty())
            {
                auto tex = ome::Texture::load(texture_path);
                tex->set_wrap({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });
                tex->set_filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                gk.sprite.emplace(ome::Sprite{ .texture = std::move(tex) });
            }
        }
        catch (const std::exception &e)
        {
            gk.mesh.reset();
            gk.sprite.reset();

            log(std::format(
                "Failed loading wizard `{}`: {}.", mesh_fbx.string(), e.what()));
        }
    }

    void
    render_()
    {
        ensure_wizard_mesh_loaded_();

        auto &gk = wizard_gk_draw_slot();

        if (!gk.mesh)
        {
            return;
        }

        auto draw_tr = transform<ome::Space::World>();

        ome::open_gl::MeshRenderTask{
            .mesh             = *gk.mesh,
            .transform        = draw_tr,
            .sprite           = gk.sprite,
            .modulate         = std::nullopt,
            .texture_env_mode = GL_MODULATE,
        }();
    }

  public:
    explicit WizardGoalkeeperNode(const Configuration &config)
        : HitboxNode(hit_dims_, hit_offset_),
          spawn_(config.position)
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = spawn_; });
    }

    void
    on_mount_() override
    {
        HitboxNode::on_mount_();

        update_transform<ome::Space::Local>([&](auto &t) { t.position = spawn_; });

        auto p = transform<ome::Space::World>().position;
        log(std::format(
            "Wizard world position ({:.2f}, {:.2f}, {:.2f}) — if edits to spawn seem ignored, rebuild "
            "`game`.",
            p[0],
            p[1],
            p[2]));
    }

    void
    on_tick_() override
    {
        render_();
    }

  private:
    ome::Vec3f spawn_;
};

} // namespace soccernoid
