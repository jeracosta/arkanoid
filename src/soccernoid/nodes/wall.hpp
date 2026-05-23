#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/render_frame.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class WallNode : public ome::HitboxNode
{
  public:
    WallNode(ome::Box region)
        : HitboxNode(region.size())
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = region.center(); });
    }

    void
    on_render_(ome::RenderFrame &frame) override
    {
        auto region = hitbox<ome::Space::World>();

        auto materials = std::vector<ome::Material>{
            { .texture = textures.wall },               // 0 - visible faces
            { .texture = ome::Texture::placeholder() }, // 1 - bottom
        };

        auto material_indices = ome::BoxFaces<std::size_t>{
            .front  = 0,
            .back   = 0,
            .left   = 0,
            .right  = 0,
            .top    = 0,
            .bottom = 1,
        };

        frame.draw_commands.push_back(ome::DrawCommand::box(region, materials, material_indices));
    }
};

} // namespace soccernoid
