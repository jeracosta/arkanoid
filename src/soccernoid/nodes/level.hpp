#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/defeat_screen.hpp"
#include "soccernoid/nodes/map.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/scene_lights.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/snail.hpp"
#include "soccernoid/nodes/spikes/test_explosion.hpp"
#include "soccernoid/nodes/teapot.hpp"
#include "soccernoid/nodes/terrain.hpp"
#include "soccernoid/nodes/victory_screen.hpp"
#include "soccernoid/nodes/wall.hpp"

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

  public:
    void
    on_mount_() override
    {
        configure_(standard_level_);

        hold(game()->events.bind([this](const PlayerDefeated &)
        {
            if (game_over_)
            {
                return;
            }
            game_over_    = true;
            pending_swap_ = PendingSwap::Defeat;
        }));

        hold(game()->events.bind([this](const PlayerVictorious &)
        {
            if (game_over_)
            {
                return;
            }
            game_over_    = true;
            pending_swap_ = PendingSwap::Victory;
        }));
    }

    void
    on_tick_() override
    {
        if (pending_swap_ == PendingSwap::None)
        {
            return;
        }

        auto swap = pending_swap_;
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
        level.emplace_child<SkyboxNode>().rename("Skybox");
        level.emplace_child<CometNode>().rename("Comet");

        level.emplace_child<spikes::TestExplosionNode>();

        level.emplace_child<MapNode>(MapNode::Configuration{
            .column_count = 4,
            .area         = { 6, 7 },
        }).rename("Map");

        // Teapot at forward side
        {
            constexpr float scale  = 0.5f;
            auto            mesh   = static_cast<std::shared_ptr<ome::Mesh>>(meshes.teapot);
            auto            size   = mesh->size();
            auto            center = mesh->center();

            float teapot_y = scale * (size[1] / 2.0f - center[1] + 1.0f);

            float forward_local_x = size[0] / 2.0f + center[0];
            float teapot_z        = -7.0f + 0.5f + scale * forward_local_x;

            auto orientation = ome::Orientation{}.steer_yaw(-std::numbers::pi_v<float> / 2);
            level.emplace_child<TeapotNode>()
                .scale({ scale })
                .position({ 0.0f, teapot_y, teapot_z })
                .orientation(orientation);
        }

        // Player at backward border (centered)
        constexpr float player_y = 1.5f;
        level.emplace_child<PlayerNode>(PlayerNode::Configuration::make_harry())
            .position({ 0.0f, player_y, 7.0f })
            .rename("Harry");

        level.emplace_child<SnailNode>().position({ -3.0f, 0.0f, -3.0f }).rename("Snail");
    } };
}

inline Level
Level::defeat()
{
    return { [](LevelNode &level)
    {
        level.emplace_child<SceneLightsNode>().rename("Lights");
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
        level.emplace_child<SceneLightsNode>().rename("Lights");
        level.emplace_child<SkyboxNode>(textures.skybox.dawn).rename("Skybox");
        level.emplace_child<CometNode>().rename("Comet");
        level.emplace_child<VictoryScreenNode>().rename("VictoryScreen");
    } };
}

} // namespace soccernoid
