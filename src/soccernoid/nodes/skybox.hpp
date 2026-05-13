#pragma once

#include <array>

#include <GL/gl.h>

#include "oh-my-engine/node.hpp"
#include "soccernoid/constants.hpp"

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
    struct Vertex
    {
        float x;
        float y;
        float z;
        float u;
        float v;
    };

    struct Face
    {
        const TexturePalette::Item *texture;
        std::array<Vertex, 4>       vertices;
    };

    static constexpr float side_ = 2 * fog.end;

    static const TexturePalette::SkyboxFaces &
    active_skybox_()
    {
        return textures.skybox.blink;
    }

    static void
    draw_face_(const Face &face)
    {
        face.texture->wrap({ GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE });
        ome::open_gl::glBindTexture(face.texture->get());

        glBegin(GL_QUADS);
        for (const auto &vertex : face.vertices)
        {
            glTexCoord2f(vertex.u, vertex.v);
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        glEnd();
    }

    static std::array<Face, 6>
    faces_(float h)
    {
        auto &skybox = active_skybox_();

        return { {
            { .texture = &skybox.front,
              .vertices = { Vertex{ -h, h, h, 0.0f, 0.0f },
                            Vertex{ h, h, h, 1.0f, 0.0f },
                            Vertex{ h, -h, h, 1.0f, 1.0f },
                            Vertex{ -h, -h, h, 0.0f, 1.0f } } },
            { .texture = &skybox.back,
              .vertices = { Vertex{ h, h, -h, 0.0f, 0.0f },
                            Vertex{ -h, h, -h, 1.0f, 0.0f },
                            Vertex{ -h, -h, -h, 1.0f, 1.0f },
                            Vertex{ h, -h, -h, 0.0f, 1.0f } } },
            { .texture = &skybox.left,
              .vertices = { Vertex{ -h, h, -h, 0.0f, 0.0f },
                            Vertex{ -h, h, h, 1.0f, 0.0f },
                            Vertex{ -h, -h, h, 1.0f, 1.0f },
                            Vertex{ -h, -h, -h, 0.0f, 1.0f } } },
            { .texture = &skybox.right,
              .vertices = { Vertex{ h, h, h, 0.0f, 0.0f },
                            Vertex{ h, h, -h, 1.0f, 0.0f },
                            Vertex{ h, -h, -h, 1.0f, 1.0f },
                            Vertex{ h, -h, h, 0.0f, 1.0f } } },
            { .texture = &skybox.top,
              .vertices = { Vertex{ -h, h, -h, 0.0f, 0.0f },
                            Vertex{ h, h, -h, 1.0f, 0.0f },
                            Vertex{ h, h, h, 1.0f, 1.0f },
                            Vertex{ -h, h, h, 0.0f, 1.0f } } },
            { .texture = &skybox.bottom,
              .vertices = { Vertex{ -h, -h, h, 0.0f, 0.0f },
                            Vertex{ h, -h, h, 1.0f, 0.0f },
                            Vertex{ h, -h, -h, 1.0f, 1.0f },
                            Vertex{ -h, -h, -h, 0.0f, 1.0f } } },
        } };
    }

    void
    render_cube_() const
    {
        constexpr float h = (side_ / 2.0f);
        auto            faces = faces_(h);

        glEnable(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        for (const auto &face : faces)
        {
            draw_face_(face);
        }

        glDisable(GL_TEXTURE_2D);
    }
};

} // namespace soccernoid
