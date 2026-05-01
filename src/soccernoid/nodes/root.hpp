#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "soccernoid/nodes/camera_control.hpp"
#include "soccernoid/nodes/frame_rate.hpp"
#include "soccernoid/nodes/time_speed.hpp"

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
            .add<ome::Slowed<FrameRateNode, 1.0f>>().named("FPS").up()
            .add<TimeSpeedNode>().named("TimeSpeed").up();
        // clang-format on
    }

    void
    on_ready_() override
    {
        log("¡Si capitán, estamos listos!");
        log("Node tree: \n" + tree_string(*this));
    }
};

} // namespace soccernoid
