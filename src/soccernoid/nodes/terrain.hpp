#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class TerrainNode : public ome::HitboxNode
{
  private:
    ome::Box mesh_region_;

    // The hitbox is 10x deeper than the visible mesh to catch collisions of fast falling
    // projectiles that would otherwise tunnel through the thin floor in a single frame.
    static ome::Box
    deepened_hitbox_(const ome::Box &mesh_region)
    {
        auto  min   = mesh_region.min();
        auto  max   = mesh_region.max();
        float depth = (max[1] - min[1]) * 10.0f;

        return ome::Box::from_bounds(ome::Vec3f{ min[0], max[1] - depth, min[2] }, max);
    }

    TerrainNode(const ome::Box &mesh_region, const ome::Box &hitbox)
        : ome::HitboxNode(hitbox.size(), hitbox.center()),
          mesh_region_(mesh_region)
    {
    }

  public:
    TerrainNode(ome::Box region)
        : TerrainNode(region, deepened_hitbox_(region))
    {
    }

    void
    on_render_(ome::RenderFrame &frame) override
    {
        auto region = mesh_region_;

        auto materials = std::vector<ome::Material>{
            { .texture = textures.dirt },                           // 0
            { .texture = textures.cobblestone, .shininess = 1.0f }, // 1
            { .texture = ome::Texture::placeholder() },             // 2
        };

        auto material_indices = ome::BoxFaces<std::size_t>{
            .front  = 0,
            .back   = 0,
            .left   = 0,
            .right  = 0,
            .top    = 1,
            .bottom = 2,
        };

        frame.draw_commands.push_back(
            ome::DrawCommand::box(region, materials, material_indices, 36));
    }
};

} // namespace soccernoid
