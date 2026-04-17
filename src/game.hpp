#pragma once

#include "chronometer.hpp"
#include "input.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <memory>

class Game
{
  public:
    class Enviroment;
    class Time;
    class Window;
    struct Configuration;
    class Session;

    static void
    run(const Configuration &config);

    static std::shared_ptr<Enviroment>
    enviroment();

  private:
    Game() = default; // class is abstract: it cannot be instantiated.

    inline static std::weak_ptr<Enviroment> enviroment_;

    friend class Session;
};

class Game::Window
{
  private:
    SDL_Window   *window_;
    SDL_GLContext gl_context_;

    friend class Game::Session;

  public:
    struct Configuration
    {
        const char *title;
        glm::uvec2  size;
    };

    Window(Configuration config)
        : window_(SDL_CreateWindow(config.title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config.size.x,
                                   config.size.y,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)),
          gl_context_(SDL_GL_CreateContext(window_))
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    ~Window()
    {
        SDL_GL_DeleteContext(gl_context_);
        SDL_DestroyWindow(window_);
    }

    glm::uvec2
    size() const
    {
        int width, height;
        SDL_GetWindowSize(window_, &width, &height);
        return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
    }

    double
    aspect_ratio() const
    {
        auto size = this->size();
        return static_cast<double>(size.x) / static_cast<double>(size.y);
    }

    bool
    is_fullscreen() const
    {
        return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    void
    toggle_fullscreen() const
    {
        auto flag = is_fullscreen() ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(window_, flag);
    }
};

class Game::Time
{
  public:
    using Unit = std::chrono::duration<float, std::ratio<1>>; // seconds as float

  private:
    Chronometer<Unit>          chronometer_;
    Chronometer<Unit>::Reading current_time_;
    float                      speed_ = 1; // independent from pause state

    void
    update_()
    {
        current_time_ = chronometer_.read();
    }

    friend class Game::Session;

  public:
    float
    elapsed() const
    {
        return current_time_.elapsed;
    }

    float
    delta() const
    {
        return current_time_.delta;
    }

    float
    speed() const
    {
        return speed_;
    }

    void
    speed(float new_value)
    {
        chronometer_.speed(new_value);
        speed_ = new_value;
    }

    bool
    is_paused() const
    {
        return chronometer_.speed() == 0;
    }

    void
    toggle_pause()
    {
        if (is_paused())
        {
            chronometer_.speed(speed_);
        }
        else
        {
            chronometer_.speed(0);
        }
    }
};

struct Game::Configuration
{
    Window::Configuration window;

    // Configures the input mapper. Called once at the beginning of the session.
    std::function<void(KeyboardInputMapper &, Session &)> configure_input;

    // Called once at the beginning of the session, after initialization of all internal
    // systems, and before the main loop starts.
    std::function<void()> on_init;

    // Called once per frame.
    std::function<void(const Session &)> on_update;
};

class Game::Enviroment
{
  private:
    Enviroment()
    {
        if (SDL_Init(SDL_INIT_VIDEO))
        {
            throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    }

    friend class Game;

  public:
    ~Enviroment()
    {
        SDL_Quit();
    }
};

inline std::shared_ptr<Game::Enviroment>
Game::enviroment()
{
    if (auto enviroment = Game::enviroment_.lock())
    {
        return enviroment;
    }
    else
    {
        // note: std::make_shared cannot be used because the constructor of Enviroment is private.
        auto new_enviroment = std::shared_ptr<Enviroment>(new Enviroment{});
        Game::enviroment_   = new_enviroment;
        return new_enviroment;
    }
}

class Game::Session
{
  private:
    Session(const Game::Configuration &config)
        : config_(config),
          window(config.window)
    {
        if (config.configure_input)
        {
            config.configure_input(input_mapper_, *this);
        }

        config_.on_init();
    }

    friend void
    Game::run(const Configuration &config);

    Configuration                     config_;
    KeyboardInputMapper               input_mapper_;
    bool                              running_     = false;
    unsigned long                     frame_count_ = 0;
    std::shared_ptr<Game::Enviroment> enviroment_  = Game::enviroment();

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

        config_.on_update(*this);

        SDL_GL_SwapWindow(window.window_);

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
};

inline void
Game::run(const Configuration &config)
{
    Game::Session(config).run_();
}
