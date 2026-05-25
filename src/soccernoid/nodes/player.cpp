#include "soccernoid/nodes/player.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <ranges>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/render_frame.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/board.hpp"
#include "soccernoid/nodes/level.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

PlayerNode::Configuration
PlayerNode::Configuration::make_harry()
{
    return {
        .movement_force = 20.0f,
        .max_speed      = 10.0f,
        .speed_decay    = 1.0f,
    };
}

std::shared_ptr<ome::Mesh>
PlayerNode::character_mesh_()
{
    auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.racoon);
    mesh->recenter();
    constexpr float target_extent = 1.8f;
    auto            sz            = mesh->size();
    float           max_dim       = std::max({ sz[0], sz[1], sz[2] });
    mesh->resize(sz * (target_extent / max_dim));
    return mesh;
}

ome::Material
PlayerNode::character_material_()
{
    return {
        .color = {
            .ambient  = ome::Color::hex(0x111111FF),
            .diffuse  = ome::Color::hex(0x111111FF),
        },
        .texture = textures.racoon,
    };
}

PlayerNode::AimArrowNode_::AimArrowNode_()
{
    auto mesh = static_cast<std::shared_ptr<ome::Mesh>>(meshes.arrow);
    mesh->resize({ 0.5f, 0.5f, 0.5f });
    auto material           = ome::Material{};
    material.color.emission = ome::Color::rgb(0.0f, 1.0f, 0.0f);
    arrow_mesh_             = &emplace_child<ome::MeshNode>(mesh, material);
    arrow_mesh_->rename("ArrowMesh");
}

void
PlayerNode::AimArrowNode_::on_shoot_()
{
    auto *player = static_cast<PlayerNode *>(parent());

    auto rotation        = ome::Orientation{}.rotate(current_angle_, ome::up);
    auto shoot_direction = rotation * ome::forward;

    player->shoot(shoot_direction);
    player->aiming_ = false;

    request_unmount();
}

void
PlayerNode::AimArrowNode_::on_mount_()
{
    auto *player    = static_cast<PlayerNode *>(parent());
    player->aiming_ = true;

    hold(game()->input.bind(Action::PlayerShoot, &AimArrowNode_::on_shoot_, this));
}

void
PlayerNode::AimArrowNode_::on_tick_()
{
    current_angle_ = std::sin(game()->time.elapsed() * aim_sweep_speed_) * aim_sweep_amplitude_;

    static constexpr float arrow_offset_ = 1.5f;
    static constexpr float arrow_height_ = 0.05f;

    auto rotation = ome::Orientation{}.rotate(current_angle_, ome::up);
    auto dir      = rotation * ome::forward;

    arrow_mesh_->position(dir * arrow_offset_ + ome::up * arrow_height_);
    arrow_mesh_->orientation(rotation);
}

void
PlayerNode::on_render_(ome::RenderFrame &frame)
{
    auto transform = this->transform<ome::Space::World>();

    ome::Orientation face_align = ome::Orientation::identity();
    face_align.rotate(ome::pi, ome::up);
    transform.orientation = transform.orientation * face_align;

    transform.position += ome::Vec3f{ 0.0f, 0.1f, 0.0f };
    transform.orientation.steer_pitch(-ome::pi / 4.0f);

    character_draw_.transform = transform;
    frame.draw_commands.push_back(character_draw_);
}

void
PlayerNode::process_movement_()
{
    if (aiming_)
    {
        return;
    }

    velocity(kinematic().velocity * std::exp(-config_.speed_decay * game()->time.delta()));

    struct MoveSpecification
    {
        Action     action;
        ome::Vec3f direction;

        operator const ome::Vec3f &() const
        {
            return direction;
        }
    };

    static auto moves = std::to_array<MoveSpecification>({
        { Action::PlayerLeft, ome::left },
        { Action::PlayerRight, ome::right },
        { Action::PlayerUp, ome::up },
        { Action::PlayerDown, ome::down },
    });

    auto is_active
        = [this](const MoveSpecification &move) { return game()->input.is_pressed(move.action); };

    auto active_moves = moves | std::views::filter(is_active);

    auto raw_direction = std::ranges::fold_left(active_moves, ome::Vec3f{}, std::plus{});

    if (norm(raw_direction) == 0)
    {
        return;
    }

    auto direction = normalized(raw_direction);

    update_kinematic<ome::Space::World>([&](auto &kinematic)
    {
        kinematic.velocity += direction * config_.movement_force * game()->time.delta();

        if (norm(kinematic.velocity) > config_.max_speed)
        {
            kinematic.velocity = normalized(kinematic.velocity) * config_.max_speed;
        }
    });
}

PlayerNode::PlayerNode(const Configuration &config)
    : config_(config)
{
    // Height and depth from the raccoon, but horizontal width from the board's long side.
    auto       racoon = character_mesh_()->size();
    auto       board  = BoardNode::mesh_size();
    ome::Vec3f hitbox_size{ std::max(board[0], board[2]), racoon[1], racoon[2] };

    emplace_child<ome::HitboxNode>(hitbox_size).rename("Hitbox");
    emplace_child<BoardNode>().position({ 0.0f, -0.30f, 0.0f }).rename("Board");
    emplace_child<AimArrowNode_>().rename("AimArrow");
}

void
PlayerNode::start_aiming()
{
    if (aiming_)
    {
        return;
    }

    emplace_child<AimArrowNode_>().rename("AimArrow");
}

void
PlayerNode::shoot(ome::Vec3f direction)
{
    auto position = transform<ome::Space::World>().position;

    auto *level = ome::find_ancestor<LevelNode>(this);

    auto &projectile = level->emplace_child<ProjectileNode>();
    projectile.position(position + normalized(direction) * 1.5f);

    auto velocity = normalized(direction) * shoot_force_ + ome::up * 0.2f;

    projectile.update_kinematic<ome::Space::World>([&](auto &k) { k.velocity = velocity; });
}

void
PlayerNode::clamp_hover_height_()
{
    if (!base_height_)
    {
        base_height_ = transform<ome::Space::Local>().position[1];
    }

    const float min_y = *base_height_;
    const float max_y = *base_height_ + hover_range_;

    float y = transform<ome::Space::Local>().position[1];

    if (y < min_y || y > max_y)
    {
        update_transform<ome::Space::Local>([&](auto &t)
        { t.position[1] = std::clamp(y, min_y, max_y); });

        update_kinematic<ome::Space::Local>([](auto &k) { k.velocity[1] = 0.0f; });
    }
}

void
PlayerNode::on_tick_()
{
    process_movement_();
    ome::KinematicNode::on_tick_();
    clamp_hover_height_();
}

} // namespace soccernoid
