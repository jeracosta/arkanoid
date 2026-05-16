#include "oh-my-engine/node.hpp"
#include "soccernoid/nodes/comet.hpp"
#include "soccernoid/nodes/fire.hpp"
#include "soccernoid/nodes/human.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/projectile.hpp"
#include "soccernoid/nodes/skybox.hpp"
#include "soccernoid/nodes/spikes/vortice.hpp"
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

            cursor
                .add<TerrainNode>(
                    ome::math::Box<3>{ { -10.0f, -fog.end, -10.0f }, { 10.0f, 0.0f, 10.0f } })
                .named("Terreno")
                .up();

            auto goalkeeper = std::make_shared<HumanNode>(HumanNode::Configuration{
                .position = { 0.0f, 0.0f, -5.0f },
            });
            cursor.add(goalkeeper).named("Goalkeeper").up();

            cursor.add<ProjectileNode>().named("Projectile").up();

            cursor.add<PlayerNode>(PlayerNode::Configuration::make_harry()).named("Harry").up();

            cursor.add<spikes::VorticeNode>(ome::Vec3f{ 0.0f, -5.0f, 0.0f }).named("Vortice").up();
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
