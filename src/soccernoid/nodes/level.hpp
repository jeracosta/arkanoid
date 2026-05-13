#include "oh-my-engine/node.hpp"
#include "soccernoid/nodes/ball.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/fire.hpp"
#include "soccernoid/nodes/human.hpp"
#include "soccernoid/nodes/player.hpp"
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

            cursor.add<FireNode>(ome::Vec3f{ 3.0f, 0.0f, -3.0f }).named("Fire").up();

            auto terrain = std::make_shared<TerrainNode>();
            cursor.add(terrain).named("Terreno").up();

            auto goalkeeper = std::make_shared<HumanNode>(HumanNode::Configuration{
                .position = { 0.0f, 0.0f, -5.0f },
            });
            cursor.add(goalkeeper).named("Goalkeeper").up();

            cursor.add<BallNode>(*terrain, goalkeeper.get()).named("Ball").up();

            cursor.add<PlayerNode>(PlayerNode::Configuration::make_harry()).named("Harry").up();
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
