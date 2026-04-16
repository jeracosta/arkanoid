#pragma once

#include "chronometer.hpp"
#include "input.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class Game::Session;
class Game::Config;
class Game::Time; // Dueño del cronometro
class Game::Window; // Dueño de la ventana
Game::run(Config config) -> {Game::Session(Game(config)).run()} // Game::Game privado, Game::Session:::run privado
// Game::Session toma lugar de Game::Context. Referencia game internamente. Tiene metodos y no lambdas                                                                     
// Game::Session tiene Game::Window y Game::Time (dueño)
class Game::Session::Enviroment_ // Setupea SDL en constructor y lo limpia en destructor
class Game::global_cleanup_

class Session {
public:
    Session() {
        env = g_env.lock();
        if (!env) {
            env = std::make_shared<Environment>();
            g_env = env;
        }
    }

private:
    std::shared_ptr<Environment> env;
};


class Application
{
  public:
    struct RuntimeContext
    {
        using TimeUnit = std::chrono::duration<float, std::ratio<1>>; // Seconds as float

        // Time measured at the start of the current frame.
        struct Time : public Chronometer<TimeUnit>::Reading  { 

          float speed;
          std::function<void(float)> set_speed();

          bool is_paused;
        } time;

        // How many frames have been rendered since the application started.
        uint frame_count;

        // Triggers a graceful shutdown of the application at the end of the current frame.
        std::function<void()> stop;

        class Window
        {
          public:
            glm::uvec2
            size() const;

            double
            aspect_ratio() const;

            bool
            is_fullscreen() const;

            void
            toggle_fullscreen() const;

            WindowContext(SDL_Window *window)
                : window_(window)
            {
            }

          private:
            SDL_Window *window_;
        } window;
    };

    struct Configuration
    {
        struct
        {
            const char *title;
            glm::uvec2  size;
        } window;

        std::function<void(KeyboardInputMapper &, const RuntimeContext &)> input_setup;

        std::function<void()> init;

        std::function<void(const RuntimeContext &)> frame_logic;
    };

  public:
    Application(Configuration config);

    Application(const Application &) = delete;

    ~Application();

    void
    run();

  private:
    SDL_Window         *window_;
    SDL_GLContext       gl_context_;
    Configuration       config_;
    KeyboardInputMapper input_mapper_;
    float               speed_;
    bool                running_ = false;
};
