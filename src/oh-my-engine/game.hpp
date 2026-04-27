#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <memory>

#include "input.hpp"
#include "oh-my-engine/entity-component-system/entity_store.hpp"
#include "oh-my-engine/entity-component-system/system_store.hpp"
#include "pause.hpp"
#include "time.hpp"
#include "window.hpp"

namespace ome {

class Node; // forward declaration

class Game
{
  public:
    class Enviroment
    {
      private:
        Enviroment();

      public:
        static std::shared_ptr<Enviroment>
        instance();

        ~Enviroment();
    };

    struct Configuration
    {
        Window::Configuration window;

        // Configures the input mapper. Called once at the beginning of the session.
        std::function<void(input::InputMapper &, Game &)> configure_input = {};

        std::function<void(ecs::SystemStore &, Game &)> configure_systems = {};

        std::function<std::unique_ptr<Node>(Game &)> make_root_node = {};

        // Called once at the beginning of the session, after initialization of all internal
        // systems, and before the main loop starts.
        std::function<void(Game &)> on_init = {};

        // Called once per frame, after processing input and before updating entities.
        std::function<void(Game &)> on_update;
    };

    ~Game();

  private:
    Configuration                      config_;
    input::InputMapper                 input_mapper_;
    bool                               running_     = false;
    unsigned long                      frame_count_ = 0;
    std::shared_ptr<Enviroment>        enviroment_  = Enviroment::instance();
    std::shared_ptr<Node>              root_node_;
    std::vector<std::function<void()>> tasks_;

    Game(const Configuration &config);

    void
    mount_(std::shared_ptr<Node> node);

    void
    update_();

    void
    run_()
    {
        running_ = true;

        while (running_)
        {
            update_();
        }
    }

  public:
    Window window;

    Time time;

    Pause pause{ time };

    ecs::EntityStore entities;

    ecs::SystemStore systems;

    void
    stop()
    {
        running_ = false;
    }

    uint
    frame_count() const
    {
        return frame_count_;
    }

    float
    instant_frame_rate() const
    {
        return 1.0f / time.delta();
    }

    Node *
    root_node()
    {
        return root_node_.get();
    }

    std::shared_ptr<Node>
    unmount_tree();

    static void
    run(const Configuration &config)
    {
        Game(config).run_();
    }

    // At the end of the frame, tasks get executed in the order they were scheduled,
    // Already excecuted tasks do not repeat at the next frame unless re-scheduled.
    void
    schedule(std::function<void()> task)
    {
        tasks_.emplace_back(std::move(task));
    }
};
} // namespace ome
