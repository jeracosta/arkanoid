#pragma once

#include "oh-my-engine/game.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

class Soccernoid : public ome::Game
{
  public:
    Settings     settings;
    GameEventBus events;

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
