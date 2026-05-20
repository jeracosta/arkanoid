#pragma once

#include "oh-my-engine/game.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// Game subclass that adds:
//   - `settings`: per-game tunables exposed via the HUD.
//   - `Events`:   global bus for cross-node communication.
// Nodes reach the live Soccernoid through Soccernoid::from(node).
class Soccernoid : public ome::Game
{
  public:
    Settings settings;
    EventBus Events;

    explicit Soccernoid(const Configuration &config)
        : Game(config)
    {
    }

    static void
    run(const Configuration &config)
    {
        Soccernoid(config).run_();
    }

    static Soccernoid &
    from(const ome::Node &node)
    {
        return *static_cast<Soccernoid *>(node.game());
    }
};

} // namespace soccernoid
