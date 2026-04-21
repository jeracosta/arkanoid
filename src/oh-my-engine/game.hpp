#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <initializer_list>
#include <memory>
#include <print>

#include "input.hpp"
#include "math/vector.hpp"
#include "oh-my-engine/entity-component-system/entity_store.hpp"
#include "oh-my-engine/entity-component-system/system_store.hpp"
#include "pause.hpp"
#include "time.hpp"
#include "window.hpp"

namespace ome {

class Game
{
  public:
    class Enviroment
    {
      private:
        Enviroment()
        {
            if (SDL_Init(SDL_INIT_VIDEO))
            {
                throw std::runtime_error(std::string("Failed to initialize SDL: ")
                                         + SDL_GetError());
            }

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        }

      public:
        static std::shared_ptr<Enviroment>
        instance()
        {
            static std::weak_ptr<Enviroment> enviroment;

            if (auto ref = enviroment.lock())
            {
                return ref;
            }
            else
            {
                // note: std::make_shared can't used because Enviroment constructor is private.
                auto new_enviroment = std::shared_ptr<Enviroment>(new Enviroment{});
                enviroment          = new_enviroment;
                return new_enviroment;
            }
        }

        ~Enviroment()
        {
            SDL_Quit();
        }
    };

    struct Configuration
    {
        Window::Configuration window;

        // Configures the input mapper. Called once at the beginning of the session.
        std::function<void(input::InputMapper &, Game &)> configure_input = {};

        std::function<void(ecs::SystemStore &, Game &)> configure_systems = {};

        // Called once at the beginning of the session, after initialization of all internal
        // systems, and before the main loop starts.
        std::function<void(Game &)> on_init = {};

        // Called once per frame.
        std::function<void(Game &)> on_update;
    };

  private:
    Game(const Configuration &config)
        : config_(config),
          window(config.window)
    {
        if (config_.configure_input)
        {
            config_.configure_input(input_mapper_, *this);
        }

        if (config_.configure_systems)
        {
            config_.configure_systems(ecs.systems, *this);
        }

        if (config_.on_init)
        {
            config_.on_init(*this);
        }
    }

    friend void
    run(const Configuration &config);

    Configuration               config_;
    input::InputMapper          input_mapper_;
    bool                        running_     = false;
    unsigned long               frame_count_ = 0;
    std::shared_ptr<Enviroment> enviroment_  = Enviroment::instance();

    void
    update_()
    {
        time.update_();

        auto event = SDL_Event{};
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running_ = false;
            }
            input_mapper_.handle(event);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        for (auto entity : ecs.entities.living_entities())
        {
            ecs.systems.update(entity);
        }

        config_.on_update(*this);

        SDL_GL_SwapWindow(window);

        frame_count_++;
    }

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

    struct
    {
        ecs::EntityStore entities = {};
        ecs::SystemStore systems;
    } ecs;

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

    static void
    run(const Configuration &config)
    {
        Game(std::move(config)).run_();
    }
};
}; // namespace ome
