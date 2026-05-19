#pragma once

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/skybox.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class SkyboxNode : public ome::Node
{
  public:
    void
    on_tick_() override
    {
        render_();
    }

  private:
    void
    render_()
    {
        ome::open_gl::render({
            .size = game()->camera.far(),
            .textures = {
                .front  = skybox_.front,
                .back   = skybox_.back,
                .left   = skybox_.left,
                .right  = skybox_.right,
                .top    = skybox_.top,
                .bottom = skybox_.bottom,
            },
        });
    }

    TexturePalette::SkyboxFaces skybox_ = textures.skybox.blink;
};

} // namespace soccernoid
