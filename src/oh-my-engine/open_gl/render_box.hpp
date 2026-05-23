#pragma once

#include <GL/gl.h>
#include <algorithm>

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome::open_gl {

struct BoxRenderTask
{
    Box world_region;

    GLenum texture_env_mode = GL_MODULATE;

    int top_subdiv_x = 1;
    int top_subdiv_z = 1;

    struct
    {
        Sprite front;
        Sprite back;
        Sprite left;
        Sprite right;
        Sprite top;
        Sprite bottom;
    } sprites;

    void
    operator()() const
    {
        const auto  bounds = world_region.bounds();
        const auto &mn     = bounds.min;
        const auto &mx     = bounds.max;

        const GLint prev_env_mode = []
        {
            GLint mode = GL_MODULATE;
            glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);
            return mode;
        }();

        auto draw_face = [&](const Sprite   &sprite,
                             math::Vector<3> a,
                             math::Vector<3> b,
                             math::Vector<3> c,
                             math::Vector<3> d,
                             math::Vector<3> normal)
        {
            ::glBindTexture(GL_TEXTURE_2D, sprite.texture->id());

            const auto &uv0 = sprite.uv_region.min();
            const auto &uv1 = sprite.uv_region.max();

            glBegin(GL_QUADS);
            {
                glNormal3f(normal[0], normal[1], normal[2]);
                glTexCoord2f(uv0[0], uv0[1]);
                glVertex3f(a[0], a[1], a[2]);
                glTexCoord2f(uv1[0], uv0[1]);
                glVertex3f(b[0], b[1], b[2]);
                glTexCoord2f(uv1[0], uv1[1]);
                glVertex3f(c[0], c[1], c[2]);
                glTexCoord2f(uv0[0], uv1[1]);
                glVertex3f(d[0], d[1], d[2]);
            }
            glEnd();
        };

        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, static_cast<GLint>(texture_env_mode));

        // +z front
        draw_face(sprites.front,
                  { mn[0], mn[1], mx[2] },
                  { mx[0], mn[1], mx[2] },
                  { mx[0], mx[1], mx[2] },
                  { mn[0], mx[1], mx[2] },
                  { 0.0f, 0.0f, 1.0f });

        // -z back
        draw_face(sprites.back,
                  { mx[0], mn[1], mn[2] },
                  { mn[0], mn[1], mn[2] },
                  { mn[0], mx[1], mn[2] },
                  { mx[0], mx[1], mn[2] },
                  { 0.0f, 0.0f, -1.0f });

        // -x left
        draw_face(sprites.left,
                  { mn[0], mn[1], mn[2] },
                  { mn[0], mn[1], mx[2] },
                  { mn[0], mx[1], mx[2] },
                  { mn[0], mx[1], mn[2] },
                  { -1.0f, 0.0f, 0.0f });

        // +x right
        draw_face(sprites.right,
                  { mx[0], mn[1], mx[2] },
                  { mx[0], mn[1], mn[2] },
                  { mx[0], mx[1], mn[2] },
                  { mx[0], mx[1], mx[2] },
                  { 1.0f, 0.0f, 0.0f });

        // +y top (optional UV subdivided grid for more lighting samples per face)
        {
            const int sx = (std::max)(1, top_subdiv_x);
            const int sz = (std::max)(1, top_subdiv_z);

            ::glBindTexture(GL_TEXTURE_2D, sprites.top.texture->id());

            const auto &uv0 = sprites.top.uv_region.min();
            const auto &uv1 = sprites.top.uv_region.max();

            const float x_extent = (std::max)(mx[0] - mn[0], 1e-6f);
            const float z_extent = (std::max)(mx[2] - mn[2], 1e-6f);

            glNormal3f(0.0f, 1.0f, 0.0f);
            glBegin(GL_QUADS);
            for (int j = 0; j < sz; ++j)
            {
                for (int i = 0; i < sx; ++i)
                {
                    const float s0 = static_cast<float>(i) / static_cast<float>(sx);
                    const float s1 = static_cast<float>(i + 1) / static_cast<float>(sx);
                    const float t0 = static_cast<float>(j) / static_cast<float>(sz);
                    const float t1 = static_cast<float>(j + 1) / static_cast<float>(sz);

                    const float x0 = mn[0] + s0 * x_extent;
                    const float x1 = mn[0] + s1 * x_extent;
                    const float z0 = mn[2] + t0 * z_extent;
                    const float z1 = mn[2] + t1 * z_extent;

                    const float u0 = uv0[0] + s0 * (uv1[0] - uv0[0]);
                    const float u1 = uv0[0] + s1 * (uv1[0] - uv0[0]);

                    const auto v_for_z = [&](float z)
                    { return uv0[1] + ((mx[2] - z) / z_extent) * (uv1[1] - uv0[1]); };

                    const float v0 = v_for_z(z0);
                    const float v1 = v_for_z(z1);

                    glTexCoord2f(u0, v1);
                    glVertex3f(x0, mx[1], z1);
                    glTexCoord2f(u1, v1);
                    glVertex3f(x1, mx[1], z1);
                    glTexCoord2f(u1, v0);
                    glVertex3f(x1, mx[1], z0);
                    glTexCoord2f(u0, v0);
                    glVertex3f(x0, mx[1], z0);
                }
            }
            glEnd();
        }

        // -y bottom
        draw_face(sprites.bottom,
                  { mn[0], mn[1], mn[2] },
                  { mx[0], mn[1], mn[2] },
                  { mx[0], mn[1], mx[2] },
                  { mn[0], mn[1], mx[2] },
                  { 0.0f, -1.0f, 0.0f });

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, prev_env_mode);
        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace ome::open_gl
