#include "oh-my-engine/node.hpp"

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
        levels_.push_back({
            [](ome::Node::CompositionCursor cursor) { cursor.add<Node>().named("Dummy"); },
        });
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
