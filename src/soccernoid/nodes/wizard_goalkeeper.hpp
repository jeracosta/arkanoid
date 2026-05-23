#pragma once

#include <format>

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

class WizardGoalkeeperNode : public ome::HitboxNode
{
  public:
    struct Configuration
    {
        ome::Vec3f position = { 0.0f, 0.0f, 0.0f };
    };

  private:
    static auto
    mesh_()
    {
        auto &gk = wizard_gk_draw_slot();

        if (gk.tried_load)
        {
            return;
        }

        gk.tried_load = true;

        try
        {
            gk.mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.wizard);
            gk.mesh->recenter();

            {
                constexpr float goalie_scale = 3.0f;
                auto            sz           = gk.mesh->size();
                float           max_dim      = std::max({ sz[0], sz[1], sz[2] });
                gk.mesh->resize(sz * (goalie_scale / max_dim));
            }

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

            log(std::format("Failed loading wizard mesh: {}", e.what()));
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

    void
    on_render_(ome::RenderFrame &frame) override
    {
        frame.draw_commands.push_back(ome::DrawCommand{
            .mesh      = mesh_(),
            .materials = { material_() },
            .transform = transform<ome::Space::World>(),
        });
    }
};

} // namespace soccernoid
