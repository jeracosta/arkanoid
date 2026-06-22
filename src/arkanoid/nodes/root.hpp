#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "arkanoid/nodes/camera_control.hpp"
#include "arkanoid/nodes/frame_rate.hpp"
#include "arkanoid/nodes/game_control.hpp"
#include "arkanoid/nodes/level.hpp"
#include "arkanoid/nodes/render_control.hpp"
#include "arkanoid/nodes/settings_hud.hpp"
#include "arkanoid/nodes/time_speed.hpp"
#include "arkanoid/nodes/window_control.hpp"

namespace arkanoid {

class RootNode : public ome::Node
{
  public:
    RootNode()
        : Node("Root")
    {
        emplace_child<CameraControlNode>(CameraControlNode::Settings{}).rename("Camera");
        emplace_child<WindowControlNode>().rename("Window");
        emplace_child<ome::Slowed<FrameRateNode, 1.0f>>().rename("FrameRate");
        emplace_child<SettingsHudNode>().rename("Hud");
        emplace_child<RenderControlNode>().rename("RenderControl");
        emplace_child<TimeSpeedNode>().rename("TimeSpeed");
        emplace_child<GameControlNode>().rename("GameControl");
        emplace_child<LevelNode>().rename("Level");
    }

    void
    log_tree()
    {
        log("Node tree: \n" + tree_string(*this));
    }

    void
    on_mount_() override
    {
        hold(game()->input.bind(Action::PrintTree, [this] { log_tree(); }));
    }

    void
    on_ready_() override
    {
        log("Aye captain, we're ready!");

        // tree modifications during mounting get scheduled, so we shedule this print too.
        game()->schedule([this] { log_tree(); });
    }
};

} // namespace arkanoid
