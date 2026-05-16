#pragma once

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

    const Component &
    kinematic() const noexcept
    {
        return kinematic_;
    }

    void
    set_kinematic(const Component &k) noexcept
    {
        kinematic_ = k;
    }

    const Vec3f &
    velocity() const noexcept
    {
        return kinematic_.velocity;
    }

    void
    set_velocity(const Vec3f &v) noexcept
    {
        kinematic_.velocity = v;
    }

    const Vec3f &
    angular_velocity() const noexcept
    {
        return kinematic_.angular_velocity;
    }

    void
    set_angular_velocity(const Vec3f &av) noexcept
    {
        kinematic_.angular_velocity = av;
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
