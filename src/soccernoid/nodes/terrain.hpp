#pragma once

#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/texture.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class TerrainNode : public ome::HitboxNode
{
  public:
    // Futsal: penalty point (origin) to goal line (-6m), full court width (20m centered at origin)
    TerrainNode()
        : HitboxNode({ { -10.0f, 0.0f, -10.0f }, { 10.0f, 0.0f, 10.0f } })
    {
    }

    void
    on_tick_() override
    {
        render_tower_();
    }

    // TODO: Remove this. Make hitbox a child
    const ome::HitboxComponent
    hitbox_world() const noexcept
    {
        auto local     = hitbox_local();
        auto transform = world_transform();

        return ome::HitboxComponent{ transform.to_world(local.min()),
                                     transform.to_world(local.max()) };
    }

  private:
    void
    render_tower_()
    {
        const auto box = hitbox_world();

        const auto &mn = box.min();
        const auto &mx = box.max();

        const float x0 = mn[0];
        const float y0 = mn[1];
        const float z0 = mn[2];

        const float x1 = mx[0];
        const float y1 = mx[1];
        const float z1 = mx[2];

        constexpr float depth = -fog.end;

        const float width  = x1 - x0;
        const float length = z1 - z0;
        const float height = y0 - depth;

        glEnable(GL_TEXTURE_2D);

        //
        // top floor
        //

        ome::open_gl::glBindTexture(textures.floor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x0, y0, z0);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(x1, y0, z0);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(x1, y0, z1);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(x0, y0, z1);
        }
        glEnd();

        //
        // dirt walls
        //

        ome::open_gl::glBindTexture(textures.dirt);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glBegin(GL_QUADS);
        {
            // left
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x0, y0, z0);

            glTexCoord2f(length, 0.0f);
            glVertex3f(x0, y0, z1);

            glTexCoord2f(length, height);
            glVertex3f(x0, depth, z1);

            glTexCoord2f(0.0f, height);
            glVertex3f(x0, depth, z0);

            // right
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x1, y0, z1);

            glTexCoord2f(length, 0.0f);
            glVertex3f(x1, y0, z0);

            glTexCoord2f(length, height);
            glVertex3f(x1, depth, z0);

            glTexCoord2f(0.0f, height);
            glVertex3f(x1, depth, z1);

            // front
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x1, y0, z1);

            glTexCoord2f(width, 0.0f);
            glVertex3f(x0, y0, z1);

            glTexCoord2f(width, height);
            glVertex3f(x0, depth, z1);

            glTexCoord2f(0.0f, height);
            glVertex3f(x1, depth, z1);

            // back
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x0, y0, z0);

            glTexCoord2f(width, 0.0f);
            glVertex3f(x1, y0, z0);

            glTexCoord2f(width, height);
            glVertex3f(x1, depth, z0);

            glTexCoord2f(0.0f, height);
            glVertex3f(x0, depth, z0);
        }
        glEnd();

        //
        // bottom
        //

        ome::open_gl::glBindTexture(*ome::Texture::placeholder());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(x0, depth, z0);

            glTexCoord2f(width, 0.0f);
            glVertex3f(x1, depth, z0);

            glTexCoord2f(width, length);
            glVertex3f(x1, depth, z1);

            glTexCoord2f(0.0f, length);
            glVertex3f(x0, depth, z1);
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace soccernoid
