#include "oh-my-engine/input.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/input.hpp"

namespace soccernoid {

class WindowControlNode : public ome::Node
{
  private:
    void
    on_mount_() override
    {
        auto bind = [&](auto action, auto callback) { hold(game()->input.bind(action, callback)); };

        bind(Action::ToggleFullscreen, [&] { game()->window.toggle_fullscreen(); });

        bind(Action::Quit, [&] { game()->stop(); });
    }
};

} // namespace soccernoid
