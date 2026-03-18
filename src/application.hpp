#include "chronometer.hpp"
#include "inputs.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/gtc/noise.hpp>
#include <ratio>

class Application
{
  public:
    using TimeUnit = std::chrono::duration<float, std::ratio<1>>; // Seconds as float

    struct FrameContext
    {
        // Time measured at the start of the current frame.
        const Chronometer<TimeUnit>::Reading &time;

        // State of actions at the start of the current frame.
        const ActionsState &&actions;

        // Triggers a graceful shutdown of the application at the end of the current frame.
        std::function<void()> stop;
    };

    struct Configuration
    {
        struct
        {
            const char *title;
            glm::uvec2  size;
        } window;

        InputMap input_map;

        std::function<void(const FrameContext &)> frame_logic;
    };

  private:
    SDL_Window   *window_;
    SDL_GLContext gl_context_;
    bool          running_;
    Configuration config_;

  public:
    Application(Configuration config)
        : config_(std::move(config))
    {
        if (SDL_Init(SDL_INIT_VIDEO))
        {
            fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
            std::exit(EXIT_FAILURE);
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
    }

    Application(const Application &) = delete;

    ~Application()
    {
        SDL_Quit();
    }

    void
    run()
    {
        window_ = SDL_CreateWindow(config_.window.title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config_.window.size.x,
                                   config_.window.size.y,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        gl_context_ = SDL_GL_CreateContext(window_);

        auto chronometer = Chronometer<TimeUnit>{};

        running_ = true;

        while (running_)
        {
            auto context = FrameContext{
                .time    = chronometer.read(),
                .actions = config_.input_map.get_triggered_actions_mask(),
                .stop    = [this]() { running_ = false; },
            };

            auto event = SDL_Event{};
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    context.stop();
                }
                config_.input_map.handle(event);
            }

            config_.frame_logic(context);

            SDL_GL_SwapWindow(window_);
        }

        SDL_GL_DeleteContext(gl_context_);
        SDL_DestroyWindow(window_);
    }
};
