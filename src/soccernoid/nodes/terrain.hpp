#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "oh-my-engine/texture.hpp"
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
    on_tick_() override
    {
        render_tower_();
    }

  private:
    void
    render_tower_()
    {
        auto region = hitbox<ome::Space::World>();

        const float width  = region.width();
        const float height = region.height();
        const float length = region.length();

        using Bounds = ome::Rect::Bounds;

        ome::open_gl::BoxRenderTask
        {
            .world_region = region,
          .sprites =
          {
              .front  = { textures.dirt,               Bounds{ { 0.0f, 0.0f }, { width,  height } } },
              .back   = { textures.dirt,               Bounds{ { 0.0f, 0.0f }, { width,  height } } },
              .left   = { textures.dirt,               Bounds{ { 0.0f, 0.0f }, { length, height } } },
              .right  = { textures.dirt,               Bounds{ { 0.0f, 0.0f }, { length, height } } },
              .top    = { textures.floor,              Bounds{ { 0.0f, 0.0f }, { 1.0f,   1.0f   } } },
              .bottom = { ome::Texture::placeholder(), Bounds{ { 0.0f, 0.0f }, { width,  length } } },
          },
        }();
    }
};

} // namespace soccernoid
