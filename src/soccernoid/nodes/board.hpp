#pragma once

#include <algorithm>
#include <memory>

#include "oh-my-engine/blend_mode.hpp"
#include "oh-my-engine/color.hpp"
#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class BoardNode : public ome::MeshNode
{
  public:
    class HoverParticles : public ome::ParticleEmitterNode
    {
      private:
        static inline const ome::ParticleScheme scheme_ = {
            .initial_velocity = ome::down * 2.0f,

            .time_to_live = 0.5f,

            .acceleration = ome::up * 4.0f,

            .color = ome::Interpolation{ ome::Color::white(), ome::Color::transparent() },

            .scale = ome::Interpolation{ 0.05f, 0.25f },

            .blend_mode = ome::BlendMode::additive(),

        };

        static inline const ome::ParticleEmitterNode::Settings settings_ = {
            .particle_blueprint = scheme_,
            .trigger_rate       = 10,
        };

      public:
        HoverParticles()
            : ome::ParticleEmitterNode(settings_)
        {
        }
    };

  private:
    static constexpr float side_offset_  = 0.5f;
    static constexpr float below_offset_ = 0.05f;

    static std::shared_ptr<ome::Mesh>
    board_mesh_()
    {
        auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.skateboard);
        mesh->recenter();

        constexpr float target_extent = 1.5f;
        auto            size          = mesh->size();
        float           max_dim       = std::max({ size[0], size[1], size[2] });
        mesh->resize(size * (target_extent / max_dim));

        mesh->rotate(ome::Orientation()
                         .steer_pitch(-ome::pi / 2.0f)
                         .steer_roll(-ome::pi / 2.0f)
                         .steer_yaw(-ome::pi / 2.0f));

        return mesh;
    }

    static ome::Material
    board_material_()
    {
        // Untextured violet, evoking the Back to the Future hoverboard.
        ome::Material material;
        material.color.ambient = ome::Color::rgb(240, 0, 80);
        material.color.diffuse = ome::Color::rgb(120, 120, 150);
        material.shininess     = 1.0f;
        return material;
    }

  public:
    BoardNode()
        : ome::MeshNode(board_mesh_(), board_material_())
    {
        emplace_child<HoverParticles>()
            .position({ -side_offset_, -below_offset_, 0.0f })
            .rename("LeftHover");

        emplace_child<HoverParticles>()
            .position({ side_offset_, -below_offset_, 0.0f })
            .rename("RightHover");
    }
};

} // namespace soccernoid
