#pragma once

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome::open_gl {

struct BoxRenderTask
{
    Box world_region;

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
        const auto &[min, max] = world_region.bounds();

        auto draw_face = [&](const Sprite   &sprite,
                             math::Vector<3> a,
                             math::Vector<3> b,
                             math::Vector<3> c,
                             math::Vector<3> d)
        {
            open_gl::glBindTexture(*sprite.texture);

            const auto &uv0 = sprite.uv_region.min();
            const auto &uv1 = sprite.uv_region.max();

            glBegin(GL_QUADS);
            {
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

        // +z front
        draw_face(sprites.front,
                  { min[0], min[1], max[2] },
                  { max[0], min[1], max[2] },
                  { max[0], max[1], max[2] },
                  { min[0], max[1], max[2] });

        // -z back
        draw_face(sprites.back,
                  { max[0], min[1], min[2] },
                  { min[0], min[1], min[2] },
                  { min[0], max[1], min[2] },
                  { max[0], max[1], min[2] });

        // -x left
        draw_face(sprites.left,
                  { min[0], min[1], min[2] },
                  { min[0], min[1], max[2] },
                  { min[0], max[1], max[2] },
                  { min[0], max[1], min[2] });

        // +x right
        draw_face(sprites.right,
                  { max[0], min[1], max[2] },
                  { max[0], min[1], min[2] },
                  { max[0], max[1], min[2] },
                  { max[0], max[1], max[2] });

        // +y top
        draw_face(sprites.top,
                  { min[0], max[1], max[2] },
                  { max[0], max[1], max[2] },
                  { max[0], max[1], min[2] },
                  { min[0], max[1], min[2] });

        // -y bottom
        draw_face(sprites.bottom,
                  { min[0], min[1], min[2] },
                  { max[0], min[1], min[2] },
                  { max[0], min[1], max[2] },
                  { min[0], min[1], max[2] });

        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace ome::open_gl
