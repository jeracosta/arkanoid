#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/fire.hpp"
#include "soccernoid/nodes/human.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/snail.hpp"
#include "soccernoid/nodes/terrain.hpp"

namespace soccernoid {

class LevelNode; // forward declaration

struct Level
{
    std::function<void(LevelNode &)> configure;

    static constexpr Level
    standard();
};

class LevelNode : public ome::Node
{
  private:
    std::vector<Level> levels_;

  public:
    LevelNode()
    {
        levels_.push_back(Level::standard());
    }

    void
    on_mount_() override
    {
        if (levels_.empty())
        {
            throw std::runtime_error(
                std::format("'{}' node was mounted without any levels.", name()));
        }
        std::invoke(levels_.front().configure, std::ref(*this));
    }
};

inline constexpr Level
Level::standard()
{
    return { [](LevelNode &level)
    {
        level.emplace_child<SkyboxNode>().rename("Skybox");

        level.emplace_child<CometNode>().rename("Comet");

        level
            .emplace_child<TerrainNode>(
                ome::Box::from_bounds({ -5.0f, -fog.end, -5.0f }, { 5.0f, 0.0f, 5.0f }))
            .rename("Terreno");

        level
            .emplace_child<HumanNode>(HumanNode::Configuration{
                .position = { 0.0f, 0.0f, -5.0f },
            })
            .rename("Goalkeeper");

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

            auto region = ome::Box::from_size_location({ size }, location);

            auto name = std::format("Pilar {}", i + 1);

            auto &terrain = level.emplace_child<TerrainNode>(region).rename(name);

            terrain.emplace_child<FireNode>().position(ome::up * size / 2).rename("Fire");
        }
    } };
}

} // namespace soccernoid
