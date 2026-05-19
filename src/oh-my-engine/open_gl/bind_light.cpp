#include <GL/gl.h>

#include "oh-my-engine/light.hpp"

namespace ome::open_gl {

void
bind(const DirectionalLight &light, GLenum slot)
{
    auto dir = Vec4<GLfloat>{ light.direction, 0 };

    auto ambient  = light.color.ambient.rgba_f();
    auto diffuse  = light.color.diffuse.rgba_f();
    auto specular = light.color.specular.rgba_f();

    glLightfv(slot, GL_POSITION, dir.data());
    glLightfv(slot, GL_AMBIENT, ambient.data());
    glLightfv(slot, GL_DIFFUSE, diffuse.data());
    glLightfv(slot, GL_SPECULAR, specular.data());
}

void
bind(const PointLight &light, GLenum slot)
{
    auto pos = Vec4<GLfloat>{ light.position, 1.0f };

    auto ambient  = light.color.ambient.rgba_f();
    auto diffuse  = light.color.diffuse.rgba_f();
    auto specular = light.color.specular.rgba_f();

    glLightfv(slot, GL_POSITION, pos.data());
    glLightfv(slot, GL_AMBIENT, ambient.data());
    glLightfv(slot, GL_DIFFUSE, diffuse.data());
    glLightfv(slot, GL_SPECULAR, specular.data());

    glLightf(slot, GL_CONSTANT_ATTENUATION, light.constant_attenuation);
    glLightf(slot, GL_LINEAR_ATTENUATION, light.linear_attenuation);
    glLightf(slot, GL_QUADRATIC_ATTENUATION, light.quadratic_attenuation);
}

void
bind(const SpotLight &light, GLenum slot)
{
    auto pos = Vec4<GLfloat>{ light.position, 1.0f };
    auto dir = Vec4<GLfloat>{ light.direction, 0.0f };

    auto ambient  = light.color.ambient.rgba_f();
    auto diffuse  = light.color.diffuse.rgba_f();
    auto specular = light.color.specular.rgba_f();

    glLightfv(slot, GL_POSITION, pos.data());
    glLightfv(slot, GL_SPOT_DIRECTION, dir.data());

    glLightfv(slot, GL_AMBIENT, ambient.data());
    glLightfv(slot, GL_DIFFUSE, diffuse.data());
    glLightfv(slot, GL_SPECULAR, specular.data());

    glLightf(slot, GL_SPOT_CUTOFF, light.cutoff_angle);
    glLightf(slot, GL_SPOT_EXPONENT, light.exponent);

    glLightf(slot, GL_CONSTANT_ATTENUATION, light.constant_attenuation);
    glLightf(slot, GL_LINEAR_ATTENUATION, light.linear_attenuation);
    glLightf(slot, GL_QUADRATIC_ATTENUATION, light.quadratic_attenuation);
}

} // namespace ome::open_gl
