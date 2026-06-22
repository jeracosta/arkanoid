#include "arkanoid/nodes/projectile.hpp"

#include <algorithm>
#include <format>
#include <limits>

#include "arkanoid/events.hpp"

namespace arkanoid {

void
ProjectileNode::bounce_from_(ome::HitboxNode &other)
{
    auto self_box  = hitbox_->hitbox<ome::Space::World>();
    auto other_box = other.hitbox<ome::Space::World>();

    auto self_center = self_box.center();
    auto self_half   = self_box.size() * 0.5f;

    constexpr float epsilon = 1e-4f;

    // Minimum translation to separate the projectile from `other`, taken per axis as the distance
    // to the nearest face. Using the raw overlap depth fails when the small projectile box ends
    // up fully inside a much larger box (the boundary walls): the overlap then equals the
    // projectile's size on every axis, the wrong axis gets picked, and it stays trapped inside.
    std::size_t penetration_axis = 0;
    float       penetration      = std::numeric_limits<float>::infinity();
    float       direction        = 1.0f;

    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        float to_positive = other_box.max()[axis] - (self_center[axis] - self_half[axis]);
        float to_negative = (self_center[axis] + self_half[axis]) - other_box.min()[axis];

        float depth = std::min(to_positive, to_negative);

        if (depth < penetration)
        {
            penetration      = depth;
            penetration_axis = axis;
            direction        = to_positive < to_negative ? 1.0f : -1.0f;
        }
    }

    if (penetration < epsilon)
    {
        return;
    }

    ome::Vec3f normal;
    normal[penetration_axis] = direction;

    // only floor bounces lose energy; side and forward wall bounces are perfectly elastic.
    float restitution = penetration_axis == 1 ? config_.elasticity : 1.0f;

    update_kinematic<ome::Space::Local>([&](ome::KinematicComponent &kinematic)
    {
        auto normal_velocity = projection(kinematic.velocity, normal);
        auto normal_speed    = norm(normal_velocity);

        if (normal_speed < config_.speed_threshold)
        {
            kinematic.velocity -= normal_velocity;
        }
        else
        {
            kinematic.velocity -= (1.0f + restitution) * normal_velocity;
        }
    });

    auto correction = normal * (penetration + epsilon);

    update_transform<ome::Space::Local>([&](auto &transform) { transform.position += correction; });
}

void
ProjectileNode::HitboxNode::on_collision_(ome::HitboxNode &other)
{
    log(std::format("Collided with {} ({})", other.name(), other.default_name()),
        ome::LogLevel::Debug);

    static_cast<ProjectileNode *>(parent())->bounce_from_(other);
}

void
ProjectileNode::on_mount_()
{
    Base_::on_mount_();
    game()->events.emit(ProjectileSpawned{});
}

void
ProjectileNode::on_unmount_()
{
    game()->events.emit(ProjectileDespawned{});
    Base_::on_unmount_();
}

} // namespace arkanoid
