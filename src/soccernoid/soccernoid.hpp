#pragma once

#include "oh-my-engine/game.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/events.hpp"

namespace soccernoid {

// Game subclass that adds a globally-accessible event bus, allowing nodes to
// communicate without holding references to one another. Nodes obtain it from
// their game pointer via Soccernoid::from(node).
class Soccernoid : public ome::Game
{
  public:
    EventBus Events;

    static void
    run(const Configuration &config)
    {
        Soccernoid game(config);
        game.init_();
        game.run_();
    }

    static Soccernoid &
    from(const ome::Node &node)
    {
        return *static_cast<Soccernoid *>(node.game());
    }

  protected:
    using ome::Game::Game;
};

} // namespace soccernoid
