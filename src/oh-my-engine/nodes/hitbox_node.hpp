#pragma once

#include "oh-my-engine/math/box.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

using HitboxComponent = math::Box<3>;

class HitboxNode : public TransformNode
{
  public:
    HitboxNode() = default;

    explicit HitboxNode(const HitboxComponent &hitbox)
        : hitbox_(hitbox)
    {
    }

    const HitboxComponent &
    hitbox_local() const noexcept
    {
        return hitbox_;
    }

    const HitboxComponent
    hitbox_world() const noexcept
    {
        auto local     = hitbox_local();
        auto transform = world_transform();

        return HitboxComponent{ transform.to_world(local.min()), transform.to_world(local.max()) };
    }

  private:
    HitboxComponent hitbox_{};
};

} // namespace ome
