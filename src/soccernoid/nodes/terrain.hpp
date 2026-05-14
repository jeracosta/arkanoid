#pragma once

#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

// TODO: Make hitbox a child
class TerrainNode : public ome::HitboxNode
{
  public:
    // Futsal: penalty point (origin) to goal line (-6m), full court width (20m centered at origin)
    TerrainNode()
        : HitboxNode({ { -10.0f, -fog.end, -10.0f }, { 10.0f, 0.0f, 10.0f } })
    {
    }

    void
    on_tick_() override
    {
        render_tower_();
    }

  private:
    const ome::math::Box<3>
    world_region_() const noexcept
    {
        auto local     = hitbox_local();
        auto transform = world_transform();

        return { transform.to_world(local.min()), transform.to_world(local.max()) };
    }

    void
    render_tower_()
    {
        const auto region = world_region_();

        const float width  = ome::math::width(region);
        const float length = ome::math::length(region);
        const float height = ome::math::height(region);

        ome::open_gl::BoxRenderTask{
          .world_region = region,
          .sprites =
          {
              .front  = { textures.dirt,                { { 0.0f, 0.0f }, { width,  height } } },
              .back   = { textures.dirt,                { { 0.0f, 0.0f }, { width,  height } } },
              .left   = { textures.dirt,                { { 0.0f, 0.0f }, { length, height } } },
              .right  = { textures.dirt,                { { 0.0f, 0.0f }, { length, height } } },
              .top    = { textures.floor,               { { 0.0f, 0.0f }, { 1.0f,   1.0f   } } },
              .bottom = { ome::Texture::placeholder(), { { 0.0f, 0.0f }, { width,  length } } },
          },
        }();
    }
};

} // namespace soccernoid
