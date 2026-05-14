#pragma once

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class SkyboxNode : public ome::Node
{
  public:
    void
    on_tick_() override
    {
        auto      position = game()->camera.position();
        GLboolean fog_was  = glIsEnabled(GL_FOG);

        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);

        glDepthMask(GL_FALSE);
        glDisable(GL_FOG);

        render_();

        if (fog_was)
        {
            glEnable(GL_FOG);
        }

        glDepthMask(GL_TRUE);
        glPopMatrix();
    }

  private:
    static constexpr float side_ = 2 * fog.end;

    void
    render_()
    {
        ome::open_gl::BoxRenderTask {
            .world_region = ome::math::Box<3>(side_),
            .sprites = {
                .front  = { skybox_.front  },
                .back   = { skybox_.back   },
                .left   = { skybox_.left   },
                .right  = { skybox_.right  },
                .top    = { skybox_.top    },
                .bottom = { skybox_.bottom },
            },
        }();
    }

    TexturePalette::SkyboxFaces skybox_ = textures.skybox.blink;
};

} // namespace soccernoid
