#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "soccernoid/nodes/camera_control.hpp"
#include "soccernoid/nodes/frame_rate.hpp"
#include "soccernoid/nodes/level.hpp"
#include "soccernoid/nodes/settings_hud.hpp"
#include "soccernoid/nodes/time_speed.hpp"
#include "soccernoid/nodes/window_control.hpp"

namespace soccernoid {

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
        emplace_child<TimeSpeedNode>().rename("TimeSpeed");
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
        log("¡Si capitán, estamos listos!");

        // tree modifications during mounting get scheduled, so we shedule this print too.
        game()->schedule([this] { log_tree(); });
    }
};

} // namespace soccernoid
