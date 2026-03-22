#pragma once

#include "application.hpp"
#include "chronometer.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

static std::optional<Application::RuntimeContext> global_runtime_context_;
const auto                                       &WORLD = global_runtime_context_; // see world.hpp

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
    assert(!global_runtime_context_.has_value()
           && "Tried to run multiple applications at the same time; global runtime context clash");

    window_ = SDL_CreateWindow(config_.window.title,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               config_.window.size.x,
                               config_.window.size.y,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    gl_context_ = SDL_GL_CreateContext(window_);

    auto chronometer = Chronometer<RuntimeContext::TimeUnit>{};

    bool stop_requested = false;

    global_runtime_context_ = { .time    = chronometer.read(),
                                .actions = config_.input_map.get_triggered_actions_mask(),
                                .stop    = [&stop_requested]() { stop_requested = true; } };

    while (!stop_requested)
    {

        auto event = SDL_Event{};
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                stop_requested = true;
            }
            config_.input_map.handle(event);
        }

        config_.frame_logic(WORLD);

        SDL_GL_SwapWindow(window_);
    }

    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
}

Application::~Application()
{
    global_runtime_context_.reset();
    SDL_Quit();
}
