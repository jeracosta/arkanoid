#pragma once

#include "oh-my-engine/sdl/event_handler.hpp"

namespace ome {

class Window; // forward declaration

// Owns the Dear ImGui context and its SDL2 + fixed-function OpenGL2 backends.
class DebugUi : public ome::sdl::EventHandler
{
  public:
    explicit DebugUi(Window &window);

    DebugUi(const DebugUi &) = delete;

    DebugUi &
    operator=(const DebugUi &)
        = delete;

    ~DebugUi();

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
