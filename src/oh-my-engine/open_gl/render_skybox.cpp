#include "oh-my-engine/skybox.hpp"

#include <GL/gl.h>

#include "oh-my-engine/open_gl/matrix_guard.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"

void
ome::open_gl::render(const Skybox &skybox)
{
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
            .world_region      = ome::Box::from_size({ skybox.size }),
            .texture_env_mode  = GL_REPLACE,
            .sprites           = {
                .front  = { skybox.textures.front  },
                .back   = { skybox.textures.back   },
                .left   = { skybox.textures.left   },
                .right  = { skybox.textures.right  },
                .top    = { skybox.textures.top    },
                .bottom = { skybox.textures.bottom },
            },
        }();

    glDepthRange(0.0, 1.0);

    glPopAttrib();
}
