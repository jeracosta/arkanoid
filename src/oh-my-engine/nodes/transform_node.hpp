#pragma once

#include <functional>

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/space.hpp"

namespace ome {

struct TransformComponent
{
    Vec3f       position    = { 0.0f };
    Orientation orientation = Orientation::identity();
    Vec3f       scale       = { 1.0f };
};

inline Vec3f
operator*(const TransformComponent &transform, const Vec3f &vector)
{
    return transform.position + transform.orientation * (transform.scale * vector);
}

inline TransformComponent
operator*(const TransformComponent &lhs, const TransformComponent &rhs)
{
    return {
        .position    = lhs * rhs.position,
        .orientation = lhs.orientation * rhs.orientation,
        .scale       = lhs.scale * rhs.scale,
    };
}

inline TransformComponent
inverse_of(const TransformComponent &transform)
{
    auto inverse_orientation = inverse_of(transform.orientation);
    auto inverse_scale       = Vec3f{ 1 } / transform.scale;
    auto inverse_position    = inverse_orientation * (inverse_scale * -transform.position);

    return {
        .position    = inverse_position,
        .orientation = inverse_orientation,
        .scale       = inverse_scale,
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

    explicit TransformNode(Vec3f local_position)
    {
        set_transform<Space::Local>({ .position = local_position });
    }

    template <Space space>
    TransformComponent
    transform() const
    {
        if constexpr (space == Space::Local)
        {
            return local_transform_;
        }
        else if constexpr (space == Space::World)
        {
            // OPTIMIZE: This is O(depth). A caching mechanism could be neccesary.

            if (auto *ancestor = find_ancestor<TransformNode>(this))
            {
                return ancestor->transform<Space::World>() * local_transform_;
            }
            return local_transform_;
        }
    };

    template <Space space>
    void
    update_transform(const std::function<void(Component &)> &fn)
    {
        static_assert(space == Space::Local, "Only updates of the local transform are supported.");
        fn(local_transform_);
    }

  private:
    template <Space space>
    void
    set_transform(const TransformComponent &transform)
    {
        if constexpr (space == Space::Local)
        {
            local_transform_ = transform;
        }
        else if constexpr (space == Space::World)
        {
            if (auto *ancestor = find_ancestor<TransformNode>(this))
            {
                local_transform_ = inverse_of(ancestor->transform<Space::World>()) * transform;
            }
            else
            {
                local_transform_ = transform;
            }
        }
    }

    Component local_transform_{};
};

} // namespace ome
