#pragma once

#include "chronometer.hpp"
#include "input.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class Application
{
  public:
    struct RuntimeContext
    {
        using TimeUnit = std::chrono::duration<float, std::ratio<1>>; // Seconds as float

        // Time measured at the start of the current frame.
        Chronometer<TimeUnit>::Reading time;

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
    bool                running_ = false;
};
