#include "oh-my-engine/node.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/fire.hpp"
#include "soccernoid/nodes/human.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/terrain.hpp"

namespace soccernoid {

struct Level
{
    std::function<void(ome::Node::CompositionCursor)> assemble_from;
};

class LevelNode : public ome::Node
{
  private:
    std::vector<Level> levels_;

  public:
    LevelNode()
    {
        levels_.push_back({ [](ome::Node::CompositionCursor cursor)
        {
            cursor.add<SkyboxNode>().named("Skybox").up();

            cursor.add<CometNode>().named("Comet").up();

            cursor.add<TerrainNode>(ome::Box{ { -5.0f, -fog.end, -5.0f }, { 5.0f, 0.0f, 5.0f } })
                .named("Terreno")
                .up();

            auto goalkeeper = std::make_shared<HumanNode>(HumanNode::Configuration{
                .position = { 0.0f, 0.0f, -5.0f },
            });
            cursor.add(goalkeeper).named("Goalkeeper").up();

            cursor.add<ProjectileNode>().named("Projectile").up();

            cursor.add<PlayerNode>(PlayerNode::Configuration::make_harry()).named("Harry").up();

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

                cursor.add<TerrainNode>(region).named(name).add<FireNode>().named("Fire").up().up();
            }
        } });
    }

    void
    on_mount_() override
    {
        if (levels_.empty())
        {
            throw std::runtime_error(
                std::format("'{}' node was mounted without any levels.", name()));
        }

        levels_.front().assemble_from(extending(*this));
    }
};

} // namespace soccernoid
