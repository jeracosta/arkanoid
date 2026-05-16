#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

class HitboxNode; // forward declaration

using Hitbox = Box;

class HitboxNode : public TransformNode
{
  public:
    HitboxNode() = default;

    explicit HitboxNode(const Vec3f &size, const Vec3f &local_position = {})
        : size_(size)
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = local_position; });
    }

    template <Space space>
    const Hitbox
    hitbox() const noexcept
    {
        auto local = Hitbox{ -size_ / 2.0f, size_ / 2.0f };

        if constexpr (space == Space::Local)
        {
            return local;
        }
        if constexpr (space == Space::World)
        {
            auto world_transform = transform<Space::World>();

            return Hitbox{ world_transform * local.min(), world_transform * local.max() };
        }
    }

  protected:
    virtual void
    on_collision_(const HitboxNode &)
    {
    }

  private:
    Vec3f size_;
};

} // namespace ome
