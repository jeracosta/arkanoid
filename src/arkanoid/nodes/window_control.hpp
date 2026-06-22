#include "oh-my-engine/input.hpp"
#include "arkanoid/input.hpp"
#include "arkanoid/nodes/arkanoid_node.hpp"
#include "arkanoid/settings.hpp"

namespace arkanoid {

class WindowControlNode : public ArkanoidNode<>
{
  private:
    using Fullscreen = settings::window::Fullscreen;

    void
    on_mount_() override
    {
        auto bind = [&](auto action, auto callback) { hold(game()->input.bind(action, callback)); };

        bind(Action::ToggleFullscreen,
             [&] { game()->settings.set<Fullscreen>(!game()->settings.get<Fullscreen>()); });

        bind(Action::Quit, [&] { game()->stop(); });

        hold(game()->settings.bind([this](const Fullscreen &fullscreen)
        { game()->window.set_fullscreen(fullscreen); }));
    }
};

} // namespace arkanoid
