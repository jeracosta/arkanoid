#pragma once

#include "arkanoid/events.hpp"
#include "arkanoid/settings.hpp"
#include "oh-my-engine/game.hpp"

namespace arkanoid {

class Arkanoid : public ome::Game
{
  public:
    Settings     settings;
    GameEventBus events;

    explicit Arkanoid(const Configuration &config)
        : Game(config)
    {
        hold(events.bind([this](const AppTerminated &) { stop(); }));
    }

    static void
    run(const Configuration &config)
    {
        Arkanoid(config).run_();
    }
};

} // namespace arkanoid
