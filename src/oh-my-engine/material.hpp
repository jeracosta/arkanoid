#pragma once

#include <GL/gl.h>
#include <memory>
#include <optional>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

struct Material
{
    struct BlendMode
    {
        GLenum source_factor      = GL_SRC_ALPHA;
        GLenum destination_factor = GL_ONE;
    };

    Color ambient;
    Color diffuse;
    Color specular;
    Color emission;
    float shininess;

    Color                            color      = Color::white();
    std::shared_ptr<Texture>         texture    = nullptr;
    std::optional<BlendMode>         blend_mode = std::nullopt;

    Material()
        : ambient(Color::rgb(0.2f, 0.2f, 0.2f)),
          diffuse(Color::rgb(0.8f, 0.8f, 0.8f)),
          specular(Color::rgb(0.0f, 0.0f, 0.0f)),
          emission(Color::rgb(0.0f, 0.0f, 0.0f)),
          shininess(0.0f)
    {
    }

    void
    apply() const
    {
        auto [ar, ag, ab, aa] = ambient.rgba_f();
        auto [dr, dg, db, da] = diffuse.rgba_f();
        auto [sr, sg, sb, sa] = specular.rgba_f();
        auto [er, eg, eb, ea] = emission.rgba_f();

        GLfloat ambient_color[]  = { ar, ag, ab, aa };
        GLfloat diffuse_color[]  = { dr, dg, db, da };
        GLfloat specular_color[] = { sr, sg, sb, sa };
        GLfloat emission_color[] = { er, eg, eb, ea };

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission_color);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
};

class MaterialGuard
{
  private:
    GLfloat ambient_[4];
    GLfloat diffuse_[4];
    GLfloat specular_[4];
    GLfloat emission_[4];
    GLfloat shininess_;

  public:
    MaterialGuard()
    {
        glGetMaterialfv(GL_FRONT, GL_AMBIENT, ambient_);
        glGetMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_);
        glGetMaterialfv(GL_FRONT, GL_SPECULAR, specular_);
        glGetMaterialfv(GL_FRONT, GL_EMISSION, emission_);
        glGetMaterialfv(GL_FRONT, GL_SHININESS, &shininess_);
    }

    ~MaterialGuard()
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission_);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess_);
    }

    MaterialGuard(const MaterialGuard &) = delete;
    MaterialGuard &
    operator=(const MaterialGuard &) = delete;
};

} // namespace ome
