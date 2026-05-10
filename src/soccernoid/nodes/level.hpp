#include "oh-my-engine/node.hpp"
#include "soccernoid/nodes/ball.hpp"
#include "soccernoid/nodes/comet.hpp"
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
            cursor.add<CometNode>().named("Comet").up();

            auto terrain = std::make_shared<TerrainNode>();
            cursor.add(terrain).named("Terreno").up();

            cursor.add<BallNode>(*terrain).named("Ball").up();
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
