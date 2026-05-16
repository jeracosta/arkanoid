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
                ome::Box{ { -5.0f, -fog.end, -5.0f }, { 5.0f, 0.0f, 5.0f } })
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

            auto location = ome::Vec3f{ std::cos(angle), 0.0f, std::sin(angle) };
            location *= pilar_distance;
            location[1] = -1.0f;

            ome::Box region = { location - ome::Vec3f{ 0.5f, 0.0f, 0.5f },
                                location + ome::Vec3f{ 0.5f, 1.0f, 0.5f } };

            auto name = std::format("Pilar {}", i + 1);

            level.emplace_child<TerrainNode>(region).rename(name).emplace_child<FireNode>().rename(
                "Fire");
        }
    } };
}

} // namespace soccernoid
