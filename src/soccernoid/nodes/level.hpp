#pragma once

#include <format>
#include <functional>
#include <memory>
#include <numbers>
#include <random>
#include <string>
#include <vector>

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/current_transformer.hpp"
#include "soccernoid/nodes/defeat_screen.hpp"
#include "soccernoid/nodes/explosive_barrel.hpp"
#include "soccernoid/nodes/map.hpp"
#include "soccernoid/nodes/moai.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/scene_light.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/snail.hpp"
#include "soccernoid/nodes/teapot.hpp"
#include "soccernoid/nodes/victory_screen.hpp"

namespace soccernoid {

class LevelNode; // forward declaration

struct Level
{
    std::function<void(LevelNode &)> configure;

    static Level
    standard();

    static Level
    defeat();

    static Level
    victory();
};

class LevelNode : public SoccernoidNode<>
{
  private:
    Level standard_level_ = Level::standard();
    Level defeat_level_   = Level::defeat();
    Level victory_level_  = Level::victory();

    enum class PendingSwap
    {
        None,
        Defeat,
        Victory,
    };

    PendingSwap pending_swap_ = PendingSwap::None;
    bool        game_over_    = false;
    bool        resetting_    = false;

    void
    configure_(const Level &level)
    {
        if (level.configure)
        {
            std::invoke(level.configure, std::ref(*this));
        }
    }

    void
    swap_to_(const Level &level)
    {
        std::vector<std::string> names;
        for (auto *child : children())
        {
            names.emplace_back(child->name());
        }

        for (const auto &name : names)
        {
            remove_child(name);
        }

        configure_(level);
    }

    void
    reset_()
    {
        resetting_    = true;
        game_over_    = false;
        pending_swap_ = PendingSwap::None;

        swap_to_(standard_level_);

        resetting_ = false;
    }

  public:
    void
    on_mount_() override
    {
        configure_(standard_level_);

        hold(game()->events.bind([this](const PlayerDefeated &)
        {
            if (resetting_)
            {
                return;
            }

            if (game_over_)
            {
                return;
            }

            game_over_    = true;
            pending_swap_ = PendingSwap::Defeat;
        }));

        hold(game()->events.bind([this](const PlayerVictorious &)
        {
            if (resetting_)
            {
                return;
            }

            if (game_over_)
            {
                return;
            }

            game_over_    = true;
            pending_swap_ = PendingSwap::Victory;
        }));

        hold(game()->input.bind(Action::Reset, [this] { reset_(); }));
    }

    void
    on_tick_() override
    {
        if (pending_swap_ == PendingSwap::None)
        {
            return;
        }

        auto swap     = pending_swap_;
        pending_swap_ = PendingSwap::None;

        if (swap == PendingSwap::Defeat)
        {
            swap_to_(defeat_level_);
        }
        else if (swap == PendingSwap::Victory)
        {
            swap_to_(victory_level_);
        }
    }
};

inline Level
Level::standard()
{
    return { [](LevelNode &level)
    {
        constexpr auto  map_area = ome::Vec2f{ 9, 10.5f };
        constexpr uint  grid_n   = 7;
        constexpr float padding  = 0.1f;

        level.emplace_child<SceneLightNode>().rename("Light");
        level.emplace_child<SkyboxNode>().rename("Skybox");
        level.emplace_child<CometNode>().rename("Comet");

        level
            .emplace_child<MapNode>(MapNode::Configuration{
                .column_count = 5,
                .area         = map_area,
            })
            .rename("Map");

        // Teapot at forward side
        {
            constexpr float scale = 0.5f;

            auto mesh   = static_cast<std::shared_ptr<ome::Mesh>>(meshes.teapot);
            auto size   = mesh->size();
            auto center = mesh->center();

            float teapot_y = scale * (size[1] / 2.0f - center[1] + 1.0f);

            float forward_local_x = size[0] / 2.0f + center[0];
            float teapot_z        = -map_area[1] + 0.5f + scale * forward_local_x;

            level.emplace_child<TeapotNode>().scale({ scale }).position(
                { 0.0f, teapot_y, teapot_z });
        }

        // Player at backward border
        {
            constexpr float player_y = 1.5f;

            level.emplace_child<PlayerNode>(PlayerNode::Configuration::make_harry())
                .position({ 0.0f, player_y, map_area[1] })
                .rename("Player");
        }

        // Grid-based obstacle placement
        {
            struct ObstacleSpawn
            {
                float                                             percentage;
                std::function<void(LevelNode &, ome::Vec3f, int)> spawn;
            };

            const std::vector<ObstacleSpawn> obstacle_spawns = {
                {
                    .percentage = 33.5f,
                    .spawn =
                        [](LevelNode &level, ome::Vec3f position, int idx)
            {
                level.emplace_child<ExplosiveBarrelNode>().position(position).rename(
                    std::format("Barrel{}", idx));
            },
                },
                {
                    .percentage = 7.5f,
                    .spawn =
                        [](LevelNode &level, ome::Vec3f position, int idx)
            {
                constexpr float phi = std::numbers::phi_v<float>;

                level.emplace_child<CurrentTransformerNode>()
                    .position(position)
                    .orientation(ome::Orientation().steer_yaw(-phi))
                    .rename(std::format("Transformer{}", idx));
            },
                },
                {
                    .percentage = 60.0f,
                    .spawn =
                        [](LevelNode &level, ome::Vec3f position, int idx)
            {
                constexpr float phi = std::numbers::phi_v<float>;

                level.emplace_child<MoaiNode>()
                    .position(position)
                    .orientation(ome::Orientation().steer_yaw(2.0f * phi))
                    .rename(std::format("Moai{}", idx));
            },
                },
            };

            std::vector<float> percentages;
            percentages.reserve(obstacle_spawns.size());

            for (const auto &obstacle_spawn : obstacle_spawns)
            {
                percentages.push_back(obstacle_spawn.percentage);
            }

            static std::mt19937                     rng{ std::random_device{}() };
            std::discrete_distribution<std::size_t> pick(percentages.begin(), percentages.end());

            constexpr float inner_min_x = -map_area[0] * (1.0f - 2.0f * padding);
            constexpr float inner_max_x = map_area[0] * (1.0f - 2.0f * padding);
            constexpr float inner_min_z = -map_area[1] * (1.0f - 2.0f * padding);
            constexpr float inner_max_z = map_area[1] * (1.0f - 2.0f * padding);

            int idx = 0;

            for (uint i = 0; i < grid_n; ++i)
            {
                for (uint j = 0; j < grid_n; ++j)
                {
                    const float x = ome::lerp(
                        inner_min_x, inner_max_x, (static_cast<float>(i) + 0.5f) / grid_n);

                    const float z = ome::lerp(
                        inner_min_z, inner_max_z, (static_cast<float>(j) + 0.5f) / grid_n);

                    const auto position = ome::Vec3f{ x, 0.0f, z };
                    const auto selected = pick(rng);

                    obstacle_spawns[selected].spawn(level, position, idx++);
                }
            }
        }

        level.emplace_child<SnailNode>().position({ -3.0f, 0.0f, -3.0f }).rename("Snail");
    } };
}

inline Level
Level::defeat()
{
    return { [](LevelNode &level)
    {
        level.emplace_child<SceneLightNode>().rename("Light");
        level.emplace_child<SkyboxNode>(textures.skybox.blood).rename("Skybox");
        level.emplace_child<CometNode>().rename("Comet");
        level.emplace_child<DefeatScreenNode>().rename("DefeatScreen");
    } };
}

inline Level
Level::victory()
{
    return { [](LevelNode &level)
    {
        level.emplace_child<SceneLightNode>().rename("Light");
        level.emplace_child<SkyboxNode>(textures.skybox.dawn).rename("Skybox");
        level.emplace_child<CometNode>().rename("Comet");
        level.emplace_child<VictoryScreenNode>().rename("VictoryScreen");
    } };
}

} // namespace soccernoid
