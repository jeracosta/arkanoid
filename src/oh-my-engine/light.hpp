#pragma once

#include <GL/gl.h>
#include <variant>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct LightColor
{
    Color ambient  = Color::rgb(0.0f, 0.0f, 0.0f);
    Color diffuse  = Color::rgb(1.0f, 1.0f, 1.0f);
    Color specular = Color::rgb(1.0f, 1.0f, 1.0f);
};

struct LightAttenuation
{
    float constant  = 1.0f;
    float linear    = 0.0f;
    float quadratic = 0.0f;
};

struct DirectionalLight
{
    LightColor color;

    Vec3f direction{ 0.0f, -1.0f, 0.0f };
};

struct PointLight
{
    LightColor       color;
    LightAttenuation attenuation;

    Vec3f position{ 0.0f, 0.0f, 0.0f };
};

struct SpotLight
{
    LightColor       color;
    LightAttenuation attenuation;

    Vec3f position{ 0.0f, 0.0f, 0.0f };
    Vec3f direction{ 0.0f, -1.0f, 0.0f };

    float cutoff_angle = 45.0f;
    float exponent     = 0.0f;
};

using Light = std::variant<DirectionalLight, PointLight, SpotLight>;

} // namespace ome

namespace ome::open_gl {

void
bind(const DirectionalLight &light, GLenum slot);

void
bind(const PointLight &light, GLenum slot);

void
bind(const SpotLight &light, GLenum slot);

inline void
bind(const Light &light, GLenum slot)
{
    std::visit([&](auto const &light) { bind(light, slot); }, light);
}

} // namespace ome::open_gl
