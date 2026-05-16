#include "light.hpp"

namespace ome {

void
DirectionalLight::apply() const
{
    auto [ar, ag, ab, aa] = ambient.rgba_f();
    auto [dr, dg, db, da] = diffuse.rgba_f();
    auto [sr, sg, sb, sa] = specular.rgba_f();

    GLfloat ambient_color[]  = { ar, ag, ab, aa };
    GLfloat diffuse_color[]  = { dr, dg, db, da };
    GLfloat specular_color[] = { sr, sg, sb, sa };

    glLightfv(gl_light_id_, GL_AMBIENT, ambient_color);
    glLightfv(gl_light_id_, GL_DIFFUSE, diffuse_color);
    glLightfv(gl_light_id_, GL_SPECULAR, specular_color);

    GLfloat position[] = {
        direction[0],
        direction[1],
        direction[2],
        0.0f
    };
    glLightfv(gl_light_id_, GL_POSITION, position);

    glEnable(gl_light_id_);
}

void
PointLight::apply() const
{
    auto [ar, ag, ab, aa] = ambient.rgba_f();
    auto [dr, dg, db, da] = diffuse.rgba_f();
    auto [sr, sg, sb, sa] = specular.rgba_f();

    GLfloat ambient_color[]  = { ar, ag, ab, aa };
    GLfloat diffuse_color[]  = { dr, dg, db, da };
    GLfloat specular_color[] = { sr, sg, sb, sa };

    glLightfv(gl_light_id_, GL_AMBIENT, ambient_color);
    glLightfv(gl_light_id_, GL_DIFFUSE, diffuse_color);
    glLightfv(gl_light_id_, GL_SPECULAR, specular_color);

    GLfloat position[] = {
        position[0],
        position[1],
        position[2],
        1.0f
    };
    glLightfv(gl_light_id_, GL_POSITION, position);

    glLightf(gl_light_id_, GL_CONSTANT_ATTENUATION, constant_attenuation);
    glLightf(gl_light_id_, GL_LINEAR_ATTENUATION, linear_attenuation);
    glLightf(gl_light_id_, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);

    glEnable(gl_light_id_);
}

void
SpotLight::apply() const
{
    auto [ar, ag, ab, aa] = ambient.rgba_f();
    auto [dr, dg, db, da] = diffuse.rgba_f();
    auto [sr, sg, sb, sa] = specular.rgba_f();

    GLfloat ambient_color[]  = { ar, ag, ab, aa };
    GLfloat diffuse_color[]  = { dr, dg, db, da };
    GLfloat specular_color[] = { sr, sg, sb, sa };

    glLightfv(gl_light_id_, GL_AMBIENT, ambient_color);
    glLightfv(gl_light_id_, GL_DIFFUSE, diffuse_color);
    glLightfv(gl_light_id_, GL_SPECULAR, specular_color);

    GLfloat position[] = {
        position[0],
        position[1],
        position[2],
        1.0f
    };
    glLightfv(gl_light_id_, GL_POSITION, position);

    GLfloat spot_direction[] = {
        direction[0],
        direction[1],
        direction[2]
    };
    glLightfv(gl_light_id_, GL_SPOT_DIRECTION, spot_direction);

    glLightf(gl_light_id_, GL_SPOT_CUTOFF, cutoff_angle);
    glLightf(gl_light_id_, GL_SPOT_EXPONENT, exponent);

    glLightf(gl_light_id_, GL_CONSTANT_ATTENUATION, constant_attenuation);
    glLightf(gl_light_id_, GL_LINEAR_ATTENUATION, linear_attenuation);
    glLightf(gl_light_id_, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);

    glEnable(gl_light_id_);
}

}
