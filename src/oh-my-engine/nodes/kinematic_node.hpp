#pragma once

#include <functional>

#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

struct KinematicComponent
{
    Vec3f velocity         = { 0.0f };
    Vec3f angular_velocity = { 0.0f };
};

class KinematicNode : public TransformNode
{
  public:
    using Component = KinematicComponent;

    KinematicNode() = default;

    explicit KinematicNode(const Component &kinematic)
        : kinematic_(kinematic)
    {
    }

    KinematicNode &
    velocity(const Vec3f &v) noexcept
    {
        kinematic_.velocity = v;
        return *this;
    }

    KinematicNode &
    angular_velocity(const Vec3f &av) noexcept
    {
        kinematic_.angular_velocity = av;
        return *this;
    }

    template <Space space = Space::Local>
    Component
    kinematic() const noexcept
    {
        if constexpr (space == Space::Local)
        {
            return kinematic_;
        }
        else
        {
            auto world             = kinematic_;
            auto orient            = transform<Space::World>().orientation;
            world.velocity         = orient * kinematic_.velocity;
            world.angular_velocity = orient * kinematic_.angular_velocity;
            return world;
        }
    }

    template <Space space = Space::Local, typename F>
    void
    update_kinematic(const F &&function)
    {
        if constexpr (space == Space::Local)
        {
            function(kinematic_);
        }
        else
        {
            auto world_orient = transform<Space::World>().orientation;

            Component world;
            world.velocity         = world_orient * kinematic_.velocity;
            world.angular_velocity = world_orient * kinematic_.angular_velocity;

            function(world);

            auto inv_rot                = inverse_of(world_orient);
            kinematic_.velocity         = inv_rot * world.velocity;
            kinematic_.angular_velocity = inv_rot * world.angular_velocity;
        }
    }

    void
    on_tick_() override
    {
        const float delta_time = Node::game()->time.delta();

        update_transform<Space::Local>([&](auto &t)
        {
            t.position += kinematic_.velocity * delta_time;
            t.orientation.steer_pitch(kinematic_.angular_velocity[0] * delta_time);
            t.orientation.steer_yaw(kinematic_.angular_velocity[1] * delta_time);
            t.orientation.steer_roll(kinematic_.angular_velocity[2] * delta_time);
        });

        TransformNode::on_tick_();
    }

  private:
    KinematicComponent kinematic_{};
};

} // namespace ome
