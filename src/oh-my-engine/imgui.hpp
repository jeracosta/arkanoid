#pragma once

#include "oh-my-engine/sdl/event_handler.hpp"

namespace ome {

class Window; // forward declaration

// Owns the Dear ImGui context and its SDL2 + fixed-function OpenGL2 backends.
class Imgui : public ome::sdl::EventHandler
{
  public:
    explicit Imgui(Window &window);

    Imgui(const Imgui &) = delete;

    Imgui &
    operator=(const Imgui &)
        = delete;

    ~Imgui();

    // Begins a new ImGui frame. Call once per frame, after events are pumped
    // and before any node builds its UI.
    void
    begin_frame();

    // Renders accumulated ImGui draw data. Call once per frame, after the scene
    // has been drawn and before swapping buffers.
    void
    end_frame();

    std::optional<SDL_Event>
    handle(const SDL_Event &event) override;
};

} // namespace ome
