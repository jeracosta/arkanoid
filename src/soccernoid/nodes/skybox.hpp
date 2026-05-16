#pragma once

#include <GL/gl.h>

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/open_gl/matrix_guard.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
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
        // TEXTURE_BIT: save/restore GL_TEXTURE_ENV_MODE. Mode is global; world BoxRenderTask sets MODULATE.
        glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_FOG_BIT | GL_TEXTURE_BIT);

        glDisable(GL_LIGHTING);

        auto matrix_guard = ome::open_gl::MatrixGuard{ GL_MODELVIEW };

        // This block removes the translation component of the modelview matrix.
        // Ensures that the skybox seems centered around the camera.
        {
            GLfloat mv[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, mv);
            mv[12] = 0.0f;
            mv[13] = 0.0f;
            mv[14] = 0.0f;
            glLoadMatrixf(mv);
        }

        glDisable(GL_FOG);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        glDepthRange(1.0, 1.0);

        ome::open_gl::BoxRenderTask{
<<<<<<< HEAD
            .world_region     = ome::math::Box<3>(game()->camera.far()),
            .texture_env_mode = GL_REPLACE,
=======
            .world_region = ome::Box(game()->camera.far()),
>>>>>>> 8c09886a2eda813ebbfd42603242681c45b994ac
            .sprites = {
                .front  = { skybox_.front  },
                .back   = { skybox_.back   },
                .left   = { skybox_.left   },
                .right  = { skybox_.right  },
                .top    = { skybox_.top    },
                .bottom = { skybox_.bottom },
            },
        }();

        glDepthRange(0.0, 1.0);

        glPopAttrib();
    }

    TexturePalette::SkyboxFaces skybox_ = textures.skybox.blink;
};

} // namespace soccernoid
