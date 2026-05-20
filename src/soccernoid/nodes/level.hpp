#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/defeat_screen.hpp"
#include "soccernoid/nodes/fire.hpp"
#include "soccernoid/nodes/fbx_character.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/nodes/wizard_goalkeeper.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/scene_lights.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/snail.hpp"
#include "soccernoid/nodes/terrain.hpp"

namespace soccernoid {

class LevelNode; // forward declaration

struct Level
{
    std::function<void(LevelNode &)> configure;

    static Level
    standard();

    static Level
    defeat();
};

class LevelNode : public SoccernoidNode<>
{
  private:
    Level standard_level_ = Level::standard();
    Level defeat_level_   = Level::defeat();

    bool swap_pending_ = false;
    bool is_defeated_  = false;

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
        // We're in on_tick_ here, so the LevelNode is Mounted (not in
        // transition). remove_child runs synchronously; the new children added
        // by configure_ also mount synchronously.
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

        hold(game()->Events.bind([this](const PlayerDefeated &)
        {
            if (is_defeated_)
            {
                return;
            }
            is_defeated_  = true;
            swap_pending_ = true;
        }));
    }

    void
    on_tick_() override
    {
        if (!swap_pending_)
        {
            return;
        }

        swap_pending_ = false;
        swap_to_(defeat_level_);
    }
};

inline Level
Level::standard()
{
    return { [](LevelNode &level)
    {
        level.emplace_child<SceneLightsNode>().rename("Lights");

        level.emplace_child<SkyboxNode>().rename("Skybox");

        level.emplace_child<CometNode>().rename("Comet");

        level
            .emplace_child<TerrainNode>(ome::Box::from_bounds(
                ome::Vec3f{ -map_half_extent, -fog.end, -map_half_extent - wall_thickness },
                ome::Vec3f{ map_half_extent, 0.0f, map_half_extent }))
            .rename("Terreno");

        // Boundary walls on three sides: left, right, and forward (the side
        // the player shoots toward). The +z side (behind the player) is left
        // open so projectiles can fall out of play. The forward wall is wider
        // than the play area to cap the corners of the lateral walls.
        // Rendered as TerrainNode (textured + collidable) rather than a bare
        // HitboxNode so they're visible.
        {
            const float h = map_half_extent;
            const float t = wall_thickness;
            const float H = wall_height;
            const float y = H * 0.5f;

            level
                .emplace_child<TerrainNode>(ome::Box::from_size_location(
                    ome::Vec3f{ t, H, 2.0f * h }, ome::Vec3f{ -h - t / 2, y, 0 }))
                .rename("WallLeft");

            level
                .emplace_child<TerrainNode>(ome::Box::from_size_location(
                    ome::Vec3f{ t, H, 2.0f * h }, ome::Vec3f{ h + t / 2, y, 0 }))
                .rename("WallRight");

            level
                .emplace_child<TerrainNode>(ome::Box::from_size_location(
                    ome::Vec3f{ 2.0f * h + 2.0f * t, H, t },
                    ome::Vec3f{ 0, y, -h - t / 2 }))
                .rename("WallForward");
        }

        level
            .emplace_child<WizardGoalkeeperNode>(WizardGoalkeeperNode::Configuration{
                .position = wizard_spawn_position,
            })
            .rename("Wizard");

        {
            constexpr float flank_x               = 1.2f;
            constexpr float toward_pitch_z        = 0.45f;
            constexpr float side_character_y       = 1.0f;
            constexpr float side_character_size    = 2.5f;
            constexpr float side_character_yaw_rad = ome::pi * 0.5f;
            const auto     &wiz                    = wizard_spawn_position;

            static const std::vector<std::filesystem::path> textures_g{
                "texture-g.png",
            };
            static const std::vector<std::filesystem::path> textures_h{
                "texture-h.png",
            };

            level
                .emplace_child<FbxCharacterNode>(FbxCharacterNode::Configuration{
                    .position          = { wiz[0] - flank_x, side_character_y, wiz[2] + toward_pitch_z },
                    .mesh_relative     = std::filesystem::path{ "characters" } / "character-g.fbx",
                    .textures_relative = textures_g,
                    .normalize_extent  = side_character_size,
                    .yaw_pi            = false,
                    .extra_yaw_rad     = side_character_yaw_rad,
                })
                .rename("G");

            level
                .emplace_child<FbxCharacterNode>(FbxCharacterNode::Configuration{
                    .position          = { wiz[0] + flank_x, side_character_y, wiz[2] + toward_pitch_z },
                    .mesh_relative     = std::filesystem::path{ "characters" } / "character-h.fbx",
                    .textures_relative = textures_h,
                    .normalize_extent  = side_character_size,
                    .yaw_pi            = false,
                    .extra_yaw_rad     = side_character_yaw_rad,
                })
                .rename("H");
        }

        level.emplace_child<ProjectileNode>().rename("Projectile");

        level.emplace_child<PlayerNode>(PlayerNode::Configuration::make_harry()).rename("Harry");

        level.emplace_child<SnailNode>().position({ -3.0f, 1.0f, -3.0f }).rename("Snail");

        static constexpr uint pilars         = 7;
        static constexpr uint pilar_distance = 13;

        for (uint i = 0; i < pilars; ++i)
        {
            float angle = (static_cast<float>(i) / pilars) * 2.0f * std::numbers::pi_v<float>;

            float size = 1.0f;

            auto location = ome::Vec3f{ std::cos(angle), 0.0f, std::sin(angle) };
            location *= pilar_distance;
            location[1] = -1.0f;

            auto region = ome::Box::from_size_location(ome::Vec3f{ size, size, size }, location);

            auto name = std::format("Pilar {}", i + 1);

            auto &terrain = level.emplace_child<TerrainNode>(region).rename(name);

            terrain.emplace_child<FireNode>().position(ome::up * size / 2).rename("Fire");
        }
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

} // namespace soccernoid
