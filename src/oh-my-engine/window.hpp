#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct WindowResized
{
    Vec2u new_size;
};

class Window : EventBus<WindowResized>
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

        resize_viewport_(size());

        emit(WindowResized{ size() });
    }

    friend void
    SDL_GL_SwapWindow(const Window &window)
    {
        return SDL_GL_SwapWindow(window.window_);
    };

    using EventBus::bind;
};

inline Vec2f
normalize(const Vec2f &window_coords, const Window &window)
{
    auto size = window.size();
    return { window_coords[0] / size[0], window_coords[1] / size[1] };
}

} // namespace ome
