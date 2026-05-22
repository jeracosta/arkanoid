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
#include "oh-my-engine/render_frame.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace {

struct WizardGkDrawSlot
{
    bool                       tried_load = false;
    std::shared_ptr<ome::Mesh> mesh{};
    ome::Material              material;
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
            gk.mesh->recenter();

            {
                constexpr float goalie_scale = 3.0f;
                auto            sz           = gk.mesh->size();
                float           max_dim      = std::max({ sz[0], sz[1], sz[2] });
                gk.mesh->resize(sz * (goalie_scale / max_dim));
            }

            if (!texture_path.empty())
            {
                auto tex = ome::Texture::load(texture_path);
                tex->set_wrap({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });
                tex->set_filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
                gk.material.texture = std::move(tex);
            }
        }
        catch (const std::exception &e)
        {
            gk.mesh.reset();
            gk.material = {};

            log(std::format(
                "Failed loading wizard `{}`: {}.", mesh_fbx.string(), e.what()));
        }
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
        on_render_(ome::RenderFrame &frame) override
        {
            ensure_wizard_mesh_loaded_();

            auto &gk = wizard_gk_draw_slot();

            if (!gk.mesh)
            {
                return;
            }

            auto draw_tr = transform<ome::Space::World>();

            frame.draw_commands.push_back(ome::DrawCommand{
                .mesh      = gk.mesh,
                .materials = { gk.material },
                .transform = draw_tr,
            });
        }

  private:
    ome::Vec3f spawn_;
};

} // namespace soccernoid
