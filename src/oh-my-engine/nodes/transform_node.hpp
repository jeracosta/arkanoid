#pragma once

#include <optional>

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
    TransformComponent result;

    result.orientation = lhs.orientation * rhs.orientation;
    result.scale       = transform(std::multiplies<>(), lhs.scale, rhs.scale);
    result.position    = lhs.to_world(rhs.position);

    return result;
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
    set_local_transform(const Component &new_transform) noexcept
    {
        local_transform_       = new_transform;
        world_transform_cache_ = std::nullopt; // invalidates cache
    }

    const Component &
    world_transform() const noexcept
    {
        if (!world_transform_cache_)
        {
            if (auto *ancestor = find_ancestor<TransformNode>(this))
            {
                world_transform_cache_ = ancestor->world_transform() * local_transform_;
            }
            else
            {
                world_transform_cache_ = local_transform_;
            }
        }

        return *world_transform_cache_;
    }

  private:
    Component                        local_transform_{};
    mutable std::optional<Component> world_transform_cache_{};
};

} // namespace ome
