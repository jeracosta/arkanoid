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
        auto local = Hitbox::from_size(size_);

        if constexpr (space == Space::Local)
        {
            return local;
        }
        if constexpr (space == Space::World)
        {
            auto world_transform = transform<Space::World>();

            return Hitbox::from_bounds(world_transform * local.min(),
                                       world_transform * local.max());
        }
    }

  protected:
    void
    on_mount_() override
    {
        game()->collision_server.register_hitbox(*this);
    }

    void
    on_unmount_() override
    {
        game()->collision_server.unregister_hitbox(*this);
    }

    virtual void
    on_collision_(HitboxNode &)
    {
    }

    void
    on_render_(RenderFrame &frame) override
    {
        frame.hitboxes.push_back(hitbox<Space::World>());
    }

  private:
    friend class CollisionServer;

    Vec3f size_;
};

} // namespace ome
