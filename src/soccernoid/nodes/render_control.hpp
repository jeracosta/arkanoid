#pragma once

#include "oh-my-engine/render_frame.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// Bridges render-related game settings into the engine's per-frame render state.
class RenderControlNode : public SoccernoidNode<>
{
  public:
    void
    on_render_(ome::RenderFrame &frame) override
    {
        frame.wireframe = game()->settings.get<settings::render::ShowWireframes>();
    }
};

} // namespace soccernoid
