#pragma once

#include "oh-my-engine/render_frame.hpp"
#include "arkanoid/nodes/arkanoid_node.hpp"
#include "arkanoid/settings.hpp"

namespace arkanoid {

// Bridges render-related game settings into the engine's per-frame render state.
class RenderControlNode : public ArkanoidNode<>
{
  public:
    void
    on_render_(ome::RenderFrame &frame) override
    {
        frame.wireframe        = game()->settings.get<settings::render::ShowWireframes>();
        frame.textures_enabled = game()->settings.get<settings::render::ShowTextures>();
        frame.smooth_shading   = game()->settings.get<settings::render::SmoothShading>();
        frame.show_hitboxes    = game()->settings.get<settings::render::ShowHitboxes>();
    }
};

} // namespace arkanoid
