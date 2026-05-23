#include "soccernoid/nodes/projectile.hpp"

#include <algorithm>
#include <cmath>

#include "soccernoid/events.hpp"
#include "soccernoid/nodes/map.hpp"
#include "soccernoid/nodes/player.hpp"

namespace soccernoid {

ome::Vec3f
ProjectileNode::position_in_map_()
{
    assert(map_);

    auto world_pos = transform<ome::Space::World>().position;
    auto map_pos   = map_->transform<ome::Space::World>().position;

    return world_pos - map_pos;
}

ome::Vec3f
ProjectileNode::force_()
{
    auto  map_local_x = position_in_map_()[0];
    float threshold_x = 0.8f * map_->area()[0];

    if (std::abs(map_local_x) > threshold_x)
    {
        float beyond     = std::abs(map_local_x) - threshold_x;
        float max_beyond = map_->area()[0] - threshold_x;
        float t          = beyond / max_beyond;
        float fx         = -std::copysign(1.0f, map_local_x) * 15.0f * t;
        return ome::Vec3f{ fx, 0.0f, 0.0f };
    }

    return { 0 };
}

void
ProjectileNode::on_tick_()
{
    Base_::on_tick_();

    update_kinematic([&](auto &k) { k.velocity += force_() * game()->time.delta(); });

    if (!wrapped_ && position_in_map_()[2] < -map_->area()[1] + 1.0f)
    {
        auto direction = kinematic().velocity;
        direction[1]   = 0.0f;
        player_->shoot(direction);
        wrapped_ = true;
    }
}

void
ProjectileNode::bounce_from_(ome::HitboxNode &other)
{
    auto hitbox        = hitbox_->hitbox<ome::Space::World>();
    auto others_hitbox = other.hitbox<ome::Space::World>();

    constexpr float epsilon = 1e-4f;

    auto overlap          = overlap_depth(hitbox, others_hitbox);
    auto penetration_axis = std::ranges::min_element(overlap) - overlap.begin();

    if (overlap[penetration_axis] < epsilon)
    {
        return;
    }

    bool is_negative
        = (hitbox.center()[penetration_axis] < others_hitbox.center()[penetration_axis]);

    ome::Vec3f normal;
    normal[penetration_axis] = is_negative ? -1.0f : 1.0f;

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
            kinematic.velocity -= (1.0f + config_.elasticity) * normal_velocity;
        }
    });

    auto correction = normal * (overlap[penetration_axis] + epsilon);

    update_transform<ome::Space::Local>([&](auto &transform) { transform.position += correction; });
}

void
ProjectileNode::HitboxNode_::on_collision_(ome::HitboxNode &other)
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

    player_ = ome::find_descendant<PlayerNode>(game()->root_node());

    if (!player_)
    {
        throw std::runtime_error("ProjectileNode couldn't find PlayerNode in the scene tree.");
    }

    map_ = ome::find_descendant<MapNode>(game()->root_node());

    if (!map_)
    {
        throw std::runtime_error("ProjectileNode couldn't find MapNode in the scene tree.");
    }
}

void
ProjectileNode::on_unmount_()
{
    game()->events.emit(ProjectileDespawned{});
    Base_::on_unmount_();
}

} // namespace soccernoid
