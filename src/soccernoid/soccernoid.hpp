#pragma once

#include "oh-my-engine/game.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// Game subclass that adds:
//   - `settings`: per-game tunables exposed via the HUD.
//   - `Events`:   global bus for cross-node communication.
// Nodes that need to reach either should derive from SoccernoidNode<TBase>
// instead of accessing through the base Game pointer.
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
};

} // namespace soccernoid
