#pragma once

#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/gl.h>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/render_frame.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

/** Loads an FBX from `assets/meshes/` with optional PNG textures under `assets/textures/`.
 *  Geometry is cached per mesh path + normalize_extent (multiple nodes can share one slot). */
class FbxCharacterNode : public ome::HitboxNode
{
  public:
    struct Configuration
    {
        ome::Vec3f                         position{};
        std::filesystem::path              mesh_relative{};
        std::vector<std::filesystem::path> textures_relative{};
        float                              normalize_extent = 2.0f;
        /** Match PlayerNode dragon facing (+π around world up). */
        bool yaw_pi = false;
        /** Extra yaw around world +Y so thin meshes aren’t edge‑on to the default third‑person camera. */
        float extra_yaw_rad = 0.0f;
        /** Same footprint as HumanNode / WizardGoalkeeperNode. */
        ome::Vec3f hitbox_size   = { 1.0f, 3.4f, 1.0f };
        ome::Vec3f hitbox_offset = { 0.0f, 0.24f, 0.0f };
    };

  private:
    struct DrawSlot
    {
        bool                       tried_load = false;
        std::shared_ptr<ome::Mesh> mesh{};
        ome::Material              material;
    };

    static DrawSlot &
    draw_slot_(const std::string &cache_key)
    {
        static std::unordered_map<std::string, DrawSlot> slots;
        return slots[cache_key];
    }

    std::string   cache_key_;
    Configuration cfg_;

    void
    ensure_mesh_loaded_()
    {
        auto &slot = draw_slot_(cache_key_);

        if (slot.tried_load)
        {
            return;
        }

        slot.tried_load = true;

        const std::filesystem::path mesh_path = FilesystemPaths::meshes / cfg_.mesh_relative;

        std::filesystem::path texture_png;
        for (const auto &rel : cfg_.textures_relative)
        {
            const auto candidate = FilesystemPaths::textures / rel;
            if (std::filesystem::exists(candidate))
            {
                texture_png = candidate;
                break;
            }
        }

        if (!std::filesystem::exists(mesh_path))
        {
            log(std::format("FBX character mesh missing (`{}`). Nothing drawn.", mesh_path.string()));
            return;
        }

        try
        {
            slot.mesh = ome::Mesh::load(mesh_path);
            slot.mesh->recenter();

            {
                auto sz       = slot.mesh->size();
                float max_dim = std::max({ sz[0], sz[1], sz[2] });
                slot.mesh->resize(sz * (cfg_.normalize_extent / max_dim));
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
                slot.material.texture = std::move(color_tex);
            }
        }
        catch (const std::exception &e)
        {
            slot.mesh.reset();
            slot.material = {};

            log(std::format(
                "Failed loading FBX `{}`: {}.", mesh_path.string(), e.what()));
        }
    }

      public:
        explicit FbxCharacterNode(Configuration cfg)
            : HitboxNode(cfg.hitbox_size, cfg.hitbox_offset),
              cache_key_(std::format(
                  "{}#{:.6g}",
                  (FilesystemPaths::meshes / cfg.mesh_relative).lexically_normal().string(),
                  cfg.normalize_extent)),
              cfg_(std::move(cfg))
        {
            update_transform<ome::Space::Local>(
                [&](auto &t) { t.position = cfg_.position; });
        }

        void
        on_render_(ome::RenderFrame &frame) override
        {
            ensure_mesh_loaded_();

            auto &slot = draw_slot_(cache_key_);

            if (!slot.mesh)
            {
                return;
            }

            auto draw_tr = transform<ome::Space::World>();

            ome::Orientation model_turn = ome::Orientation::identity();
            if (cfg_.yaw_pi)
            {
                model_turn.rotate(ome::pi, ome::up);
            }
            if (cfg_.extra_yaw_rad != 0.0f)
            {
                model_turn.rotate(cfg_.extra_yaw_rad, ome::up);
            }
            draw_tr.orientation = draw_tr.orientation * model_turn;

            frame.draw_commands.push_back(ome::DrawCommand{
                .mesh      = slot.mesh,
                .materials = { slot.material },
                .transform = draw_tr,
            });
        }
};

} // namespace soccernoid
