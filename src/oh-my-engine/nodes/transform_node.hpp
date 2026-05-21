#pragma once

#include <functional>

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/space.hpp"
#include "oh-my-engine/transform.hpp"

namespace ome {

class TransformNode : public Node
{
  public:
    using Component = Transform;

    TransformNode() = default;

    explicit TransformNode(const Component &local)
        : local_transform_(local)
    {
    }

    template <Space space>
    Transform
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

    template <Space space, typename F>
    void
    update_transform(const F &&function)
    {
        static_assert(space == Space::Local, "Only updates of the local transform are supported.");
        function(local_transform_);
    }

    TransformNode &
    position(const Vec3f &position)
    {
        update_transform<Space::Local>([&](auto &t) { t.position = position; });
        return *this;
    }

    TransformNode &
    orientation(const Orientation &orientation)
    {
        update_transform<Space::Local>([&](auto &t) { t.orientation = orientation; });
        return *this;
    }

    TransformNode &
    scale(const Vec3f &scale)
    {
        update_transform<Space::Local>([&](auto &t) { t.scale = scale; });
        return *this;
    }

  private:
    template <Space space>
    void
    set_transform(const Transform &transform)
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
