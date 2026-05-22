#include "oh-my-engine/material.hpp"

namespace ome::open_gl {

void
bind(const Material &material)
{
    auto to_gl = [&](const Color &c) -> std::array<GLfloat, 4> { return c.rgba_f(); };

    auto ambient  = to_gl(material.color.ambient);
    auto diffuse  = to_gl(material.color.diffuse);
    auto specular = to_gl(material.color.specular);
    auto emission = to_gl(material.color.emission);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.data());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.data());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.data());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission.data());
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);

    if (material.texture != nullptr)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, material.texture->id());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, static_cast<GLint>(material.env_mode));
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    if (material.blend_mode.has_value())
    {
        glEnable(GL_BLEND);
        glBlendFunc(material.blend_mode->source_factor, material.blend_mode->destination_factor);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

} // namespace ome::open_gl
