#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class TerrainNode : public ome::HitboxNode
{
  public:
    TerrainNode(ome::Box region)
        : HitboxNode(region.size())
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = region.center(); });
    }

    void
    on_render_(ome::RenderFrame &frame) override
    {
        auto region = hitbox<ome::Space::World>();

        auto materials = std::vector<ome::Material>{
            { .texture = textures.dirt },                     // 0
            { .texture = textures.floor, .shininess = 1.0f }, // 1
            {},                                               // 2
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
            ome::DrawCommand::box(region, materials, material_indices, 24));
    }
};

} // namespace soccernoid
