#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/input.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct WindowResized
{
    Vec2u new_size;
};

class Window : EventBus<WindowResized>, public ome::sdl::EventHandler
{
  private:
    SDL_Window   *window_;
    SDL_GLContext gl_context_;

    void
    resize_viewport_(Vec2u new_size)
    {
        auto &[width, height] = new_size;
        glViewport(0, 0, width, height);
    }

    void
    on_resize_(Vec2u new_size)
    {
        resize_viewport_(new_size);
        emit(WindowResized{ new_size });
    }

  public:
    struct Configuration
    {
        const char *title;
        Vec2u       size;
    };

    Window(Configuration config)
        : window_(SDL_CreateWindow(config.title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config.size[0],
                                   config.size[1],
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN)),
          gl_context_(SDL_GL_CreateContext(window_))
    {
        if (!window_)
        {
            throw std::runtime_error(std::string("Failed to create window: ") + SDL_GetError());
        }

        if (!gl_context_)
        {
            throw std::runtime_error(std::string("Failed to create OpenGL context: ")
                                     + SDL_GetError());
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_GL_SetSwapInterval(1);
    }

    Window(const Window &) = delete;

    Window &
    operator=(const Window &)
        = delete;

    Window(Window &&) = default;
    Window &
    operator=(Window &&)
        = default;

    ~Window()
    {
        SDL_GL_DeleteContext(gl_context_);
        SDL_DestroyWindow(window_);
    }

    Vec2u
    size() const
    {
        Vec2i size;
        auto &[width, height] = size;
        SDL_GetWindowSize(window_, &width, &height);
        return Vec2u(size);
    }

    double
    aspect_ratio() const
    {
        auto [width, height] = Vec2f(this->size());
        return width / height;
    }

    bool
    is_fullscreen() const
    {
        return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    void
    toggle_fullscreen()
    {
        auto flag = is_fullscreen() ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_SetWindowFullscreen(window_, flag);
    }

    friend void
    SDL_GL_SwapWindow(const Window &window)
    {
        return SDL_GL_SwapWindow(window.window_);
    };

    std::optional<SDL_Event>
    handle(const SDL_Event &event) override
    {
        if (event.type != SDL_WINDOWEVENT || event.window.event != SDL_WINDOWEVENT_RESIZED)
        {
            return event;
        }

        [[unlikely]]
        if (event.window.windowID != SDL_GetWindowID(window_))
        {
            return event;
        }

        on_resize_({ event.window.data1, event.window.data2 });

        return std::nullopt;
    }

    using EventBus::bind;
};

inline Vec2f
normalize(const Vec2f &window_coords, const Window &window)
{
    auto size = window.size();
    return { window_coords[0] / size[0], window_coords[1] / size[1] };
}

} // namespace ome
