#include <memory>

#include "oh-my-engine/math/box.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome::open_gl {

struct BoxRenderTask
{
    math::Box<3> world_region;

    // TODO: Extract Sprite
    struct Sprite
    {
        std::shared_ptr<Texture> texture;
        math::Box<2>             uv_region;
    };

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
        const auto &mn = world_region.min();
        const auto &mx = world_region.max();

        auto draw_face = [&](const Sprite   &sprite,
                             math::Vector<3> a,
                             math::Vector<3> b,
                             math::Vector<3> c,
                             math::Vector<3> d)
        {
            open_gl::glBindTexture(*sprite.texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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
                  { mn[0], mn[1], mx[2] },
                  { mx[0], mn[1], mx[2] },
                  { mx[0], mx[1], mx[2] },
                  { mn[0], mx[1], mx[2] });

        // -z back
        draw_face(sprites.back,
                  { mx[0], mn[1], mn[2] },
                  { mn[0], mn[1], mn[2] },
                  { mn[0], mx[1], mn[2] },
                  { mx[0], mx[1], mn[2] });

        // -x left
        draw_face(sprites.left,
                  { mn[0], mn[1], mn[2] },
                  { mn[0], mn[1], mx[2] },
                  { mn[0], mx[1], mx[2] },
                  { mn[0], mx[1], mn[2] });

        // +x right
        draw_face(sprites.right,
                  { mx[0], mn[1], mx[2] },
                  { mx[0], mn[1], mn[2] },
                  { mx[0], mx[1], mn[2] },
                  { mx[0], mx[1], mx[2] });

        // +y top
        draw_face(sprites.top,
                  { mn[0], mx[1], mn[2] },
                  { mx[0], mx[1], mn[2] },
                  { mx[0], mx[1], mx[2] },
                  { mn[0], mx[1], mx[2] });

        // -y bottom
        draw_face(sprites.bottom,
                  { mn[0], mn[1], mx[2] },
                  { mx[0], mn[1], mx[2] },
                  { mx[0], mn[1], mn[2] },
                  { mn[0], mn[1], mn[2] });

        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace ome::open_gl
