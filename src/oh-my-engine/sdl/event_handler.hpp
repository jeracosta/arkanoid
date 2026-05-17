#pragma once

#include <SDL_events.h>
#include <optional>

namespace ome::sdl {

class EventHandler
{
  public:
    using Result = std::optional<SDL_Event>; // empty if consumed the event

    virtual ~EventHandler() = default;

    // Handles an SDL event.
    // Can "consume" the event by returning nullopt, or pass it along by returning the event.
    virtual Result
    handle(const SDL_Event &event)
        = 0;
};

// Monadic chaining of event handlers.
// The first handler that consumes the event will stop the chain.
template <class THandler>
inline std::optional<SDL_Event>
operator|(const std::optional<SDL_Event> &event, THandler &handler)
{
    return event.has_value() ? handler.handle(*event) : std::nullopt;
}

} // namespace ome::sdl
