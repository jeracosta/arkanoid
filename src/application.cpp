#include "application.hpp"
#include "chronometer.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

Application::Application(Configuration config)
    : config_(std::move(config))
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
}

void
Application::run()
{
    assert(!running_ && "Must not run an already running application.");

    window_ = SDL_CreateWindow(config_.window.title,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               config_.window.size.x,
                               config_.window.size.y,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    gl_context_ = SDL_GL_CreateContext(window_);

    auto chronometer = Chronometer<RuntimeContext::TimeUnit>{};

    auto context = RuntimeContext{
        .time = {},
        .stop = [this] { running_ = false; },
    };

    config_.input_setup(input_mapper_, context);

    running_ = true;

    while (running_)
    {

        context.time = chronometer.read();

        auto event = SDL_Event{};
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running_ = false;
            }
            input_mapper_.handle(event);
        }

        config_.frame_logic(context);

        SDL_GL_SwapWindow(window_);
    }

    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
}

Application::~Application()
{
    SDL_Quit();
}
