#pragma once

#include <GL/gl.h>
#include <memory>

#include "oh-my-engine/blend_mode.hpp"
#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

struct Material
{
    struct
    {
        Color ambient  = Color::hex(0x333333FF);
        Color diffuse  = Color::hex(0xCCCCCCFF);
        Color specular = Color::hex(0x000000FF);
        Color emission = Color::hex(0x000000FF);
        Color base     = Color::hex(0xFFFFFFFF);
    } color;

    float shininess = 0.0f;

    std::shared_ptr<Texture> texture    = nullptr;
    TextureEnvironmentMode   env_mode   = TextureEnvironmentMode::Modulate;
    BlendMode                blend_mode = BlendMode::opaque();

    Material() = default;
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
    operator=(const MaterialGuard &)
        = delete;
};

} // namespace ome

namespace ome::open_gl {

void
bind(const Material &material);

}
