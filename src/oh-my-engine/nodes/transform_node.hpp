#pragma once

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"

namespace ome {

struct TransformComponent
{
    Vec3f       position    = { 0.0f };
    Orientation orientation = Orientation::identity();
    Vec3f       scale       = { 1.0f };

    Vec3f
    to_world(const Vec3f &local_position) const noexcept
    {
        return position + orientation * transform(std::multiplies<>(), scale, local_position);
    }

    Vec3f
    to_local(const Vec3f &world_position) const noexcept
    {
        auto delta                 = world_position - position;
        auto oriented_delta        = inverse_of(orientation) * delta;
        auto scaled_oriented_delta = transform(std::divides<>(), oriented_delta, scale);

        return scaled_oriented_delta;
    }
};

inline TransformComponent
operator*(const TransformComponent &lhs, const TransformComponent &rhs)
{
    return {
        .position    = lhs.to_world(rhs.position),
        .orientation = lhs.orientation * rhs.orientation,
        .scale       = transform(std::multiplies<>(), lhs.scale, rhs.scale),
    };
}

class TransformNode : public Node
{
  public:
    using Component = TransformComponent;

    TransformNode() = default;

    explicit TransformNode(const Component &local)
        : local_transform_(local)
    {
    }

    const Component &
    local_transform() const noexcept
    {
        return local_transform_;
    }

    void
    set_local_transform(const Component &t) noexcept
    {
        local_transform_ = t;
    }

    // OPTIMIZE: This is O(depth). A caching mechanism could be neccesary.
    const Component
    world_transform() const noexcept
    {
        if (auto *ancestor = find_ancestor<TransformNode>(this))
        {
            return ancestor->world_transform() * local_transform_;
        }
        return local_transform_;
    }

  private:
    Component local_transform_{};
};

} // namespace ome
