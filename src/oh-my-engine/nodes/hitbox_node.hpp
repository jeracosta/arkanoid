#pragma once

#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

struct HitboxComponent
{
    Vec3f min;
    Vec3f max;

    bool
    contains(const Vec3f &vector) const noexcept
    {
        return clamp(vector, min, max) == vector;
    }

    std::array<ome::Vec3f, 4>
    corners() const
    {
        return std::to_array<ome::Vec3f>({
            { min[0], min[1], min[2] },
            { max[0], min[1], min[2] },
            { max[0], min[1], max[2] },
            { min[0], min[1], max[2] },
        });
    }
};

inline bool
are_colliding(const HitboxComponent &a, const HitboxComponent &b) noexcept
{
    constexpr auto compare
        = [](auto lhs, auto rhs) { return component_wise(std::less{}, lhs.min, rhs.max); };

    return compare(a, b) && compare(b, a);
}

class HitboxNode : public TransformNode
{
  public:
    HitboxNode() = default;

    explicit HitboxNode(const HitboxComponent &local)
        : hitbox_(local)
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

        return HitboxComponent{ .min = transform.to_world(local.min),
                                .max = transform.to_world(local.max) };
    }

    bool
    is_touching(const HitboxNode &other) const noexcept
    {
        return are_colliding(hitbox_world(), other.hitbox_world());
    }

  private:
    HitboxComponent hitbox_{};
};

} // namespace ome
