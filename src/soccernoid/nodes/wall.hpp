#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

// Visible boundary wall. Same collision behavior as TerrainNode -- projectiles
// bounce off via the existing HitboxNode hookup -- but rendered with the
// dedicated wall texture on all visible faces.
class WallNode : public ome::HitboxNode
{
  public:
    WallNode(ome::Box region)
        : HitboxNode(region.size())
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = region.center(); });
    }

    void
    on_tick_() override
    {
        auto region = hitbox<ome::Space::World>();

        const float w = region.width();
        const float h = region.height();
        const float l = region.length();

        using Bounds = ome::Rect::Bounds;

        ome::open_gl::BoxRenderTask{
            .world_region = region,
            .sprites =
                {
                    .front  = { textures.wall, Bounds{ { 0.0f, 0.0f }, { w, h } } },
                    .back   = { textures.wall, Bounds{ { 0.0f, 0.0f }, { w, h } } },
                    .left   = { textures.wall, Bounds{ { 0.0f, 0.0f }, { l, h } } },
                    .right  = { textures.wall, Bounds{ { 0.0f, 0.0f }, { l, h } } },
                    .top    = { textures.wall, Bounds{ { 0.0f, 0.0f }, { w, l } } },
                    .bottom = { ome::Texture::placeholder(), Bounds{ { 0.0f, 0.0f }, { w, l } } },
                },
        }();
    }
};

} // namespace soccernoid
