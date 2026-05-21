#pragma once

#include <GL/gl.h>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome::open_gl {

inline void
render_quad(const std::array<Vec3f, 4> &vertices, const Material &material)
{
    if (material.blend_mode.has_value())
    {
        glEnable(GL_BLEND);
        glBlendFunc(material.blend_mode->source_factor, material.blend_mode->destination_factor);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (material.texture)
    {
        glEnable(GL_TEXTURE_2D);
        ::glBindTexture(GL_TEXTURE_2D, material.texture->id());
    }

    glColor(material.color.emission);

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(vertices[0][0], vertices[0][1], vertices[0][2]);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(vertices[1][0], vertices[1][1], vertices[1][2]);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(vertices[2][0], vertices[2][1], vertices[2][2]);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(vertices[3][0], vertices[3][1], vertices[3][2]);
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

} // namespace ome::open_gl
