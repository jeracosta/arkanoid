#pragma once

#include <GL/gl.h>

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/texture.hpp"

namespace soccernoid {

class SkyboxNode : public ome::Node
{
  public:
    SkyboxNode():
      Node()
    {
    }

    void
    on_tick_() override
    {
        auto  position    = game()->camera.position();
        GLboolean fog_was = glIsEnabled(GL_FOG);

        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);

        glDepthMask(GL_FALSE);
        glDisable(GL_FOG);

        render_cube_();

        if (fog_was)
        {
            glEnable(GL_FOG);
        }

        glDepthMask(GL_TRUE);
        glPopMatrix();
    }

  private:
    static constexpr float side_ = 80.0f;

    void
    render_cube_() const
    {
        constexpr float h = (side_ / 2.0f);
        constexpr float tiles = 8.0f;

        auto texture = ome::Texture::placeholder();
        texture->wrap({ GL_REPEAT, GL_REPEAT });

        glEnable(GL_TEXTURE_2D);
        ome::open_gl::glBindTexture(*texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NEAREST);

        glBegin(GL_QUADS);
        {
            // Front (+Z)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-h, h, h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(h, h, h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(h, -h, h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(-h, -h, h);

            // Back (-Z)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(h, h, -h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(-h, h, -h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(-h, -h, -h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(h, -h, -h);

            // Left (-X)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-h, h, -h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(-h, h, h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(-h, -h, h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(-h, -h, -h);

            // Right (+X)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(h, h, h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(h, h, -h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(h, -h, -h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(h, -h, h);

            // Top (+Y)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-h, h, -h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(h, h, -h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(h, h, h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(-h, h, h);

            // Bottom (-Y)
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-h, -h, h);
            glTexCoord2f(tiles, 0.0f);
            glVertex3f(h, -h, h);
            glTexCoord2f(tiles, tiles);
            glVertex3f(h, -h, -h);
            glTexCoord2f(0.0f, tiles);
            glVertex3f(-h, -h, -h);
        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace soccernoid
