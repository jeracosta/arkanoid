#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "soccernoid/nodes/camera_control.hpp"
#include "soccernoid/nodes/frame_rate.hpp"
#include "soccernoid/nodes/level.hpp"
#include "soccernoid/nodes/time_speed.hpp"
#include "soccernoid/nodes/window_control.hpp"

namespace soccernoid {

class RootNode : public ome::Node
{
  public:
    RootNode()
        : Node("Root")
    {
        // clang-format off
        extending(*this)
            .add<CameraControlNode>().named("Camera").up()
            .add<WindowControlNode>().named("Window").up()
            .add<ome::Slowed<FrameRateNode, 1.0f>>().named("FrameRate").up()
            .add<TimeSpeedNode>().named("TimeSpeed").up()
            .add<LevelNode>().named("Level").up();
        // clang-format on
    }

    void
    on_ready_() override
    {
        log("¡Si capitán, estamos listos!");

        // tree modifications during mounting get scheduled, so we shedule this print too.
        game()->schedule([this] { log("Node tree: \n" + tree_string(*this)); });
    }
};

} // namespace soccernoid
