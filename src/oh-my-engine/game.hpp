#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <memory>

#include "camera.hpp"
#include "collision_server.hpp"
#include "color.hpp"
#include "input.hpp"
#include "logger.hpp"
#include "time.hpp"
#include "window.hpp"

namespace ome {

class Node; // forward declaration

// #region Enviroment and configuration

class Game : public EventConnectionHolder
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
        struct Fog
        {
            ome::Color color   = {};
            float      start   = 25.0f;
            float      end     = 50.0f;
            bool       enabled = true;
        };

        struct Lighting
        {
            Color global_ambient = Color::rgb(0.2f, 0.2f, 0.2f);
            bool  enabled        = true;
        };

        std::function<std::unique_ptr<Logger>()> make_logger
            = [] { return std::make_unique<ConsoleLogger>(); };

        Window::Configuration window;

        Camera::Settings camera;

        Fog fog;

        Lighting lighting;

        std::function<input::InputMapper(Game &)> make_input_mapper = {};

        std::function<std::shared_ptr<Node>(Game &)> make_root_node = {};

        // Called once at the beginning of the session, after initialization of all internal
        // systems, and before the main loop starts.
        std::function<void(Game &)> on_init = {};

        // Called once per frame, after processing input and before updating entities.
        std::function<void(Game &)> on_update = {};
    };

    ~Game();

    // #endregion

    // #region Lifecycle management

  private:
    Configuration                      config_;
    bool                               running_     = false;
    unsigned long                      frame_count_ = 0;
    std::shared_ptr<Enviroment>        enviroment_  = Enviroment::instance();
    std::shared_ptr<Node>              root_node_;
    std::vector<std::function<void()>> tasks_;
    std::unique_ptr<Logger>            logger_;

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

    void
    on_projection_update_(const ProjectionUpdated &projection);

    void
    on_window_resize_(const WindowResized &projection);

    // #endregion

    void
    resolve_tasks_();

    // #region Game interface

  public:
    Window window;

    Camera camera;

    Time time;

    input::InputMapper input;

    CollisionServer collision_server;

    void
    stop()
    {
        running_ = false;
    }

    Logger &
    logger()
    {
        return *logger_;
    }

    uint
    frame_count() const
    {
        return frame_count_;
    }

    float
    instant_frame_rate() const
    {
        return 1.0f / time.unscaled.delta();
    }

    bool
    is_paused() const
    {
        return time.scale() == 0.0f;
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

    // #endregion
};
} // namespace ome
