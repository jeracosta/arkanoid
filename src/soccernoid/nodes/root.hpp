#include <GL/gl.h>

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "soccernoid/constants.hpp"
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
    log_tree()
    {
        log("Node tree: \n" + tree_string(*this));
    }

    void
    on_mount_() override
    {
        hold(game()->input.bind(Action::PrintTree, [this] { log_tree(); }));

        auto [r, g, b, a] = palette.fog.rgba();
        float fog_col[4] = { r, g, b, a };

        glClearColor(fog_col[0], fog_col[1], fog_col[2], fog_col[3]);

        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, fog.start);
        glFogf(GL_FOG_END, fog.end);
        glFogfv(GL_FOG_COLOR, fog_col);
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
