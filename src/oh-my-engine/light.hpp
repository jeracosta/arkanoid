#pragma once

#include <GL/gl.h>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Light
{
  protected:
    GLenum gl_light_id_;

  public:
    Color ambient;
    Color diffuse;
    Color specular;

    explicit Light(GLenum gl_light_id)
        : gl_light_id_(gl_light_id),
          ambient(Color::rgb(0.0f, 0.0f, 0.0f)),
          diffuse(Color::rgb(1.0f, 1.0f, 1.0f)),
          specular(Color::rgb(1.0f, 1.0f, 1.0f))
    {
    }

    virtual ~Light() = default;

    virtual void
    apply() const = 0;

    GLenum
    gl_id() const
    {
        return gl_light_id_;
    }
};

class DirectionalLight : public Light
{
  public:
    math::Vector<3> direction;

    DirectionalLight(GLenum gl_light_id, const math::Vector<3> &dir)
        : Light(gl_light_id),
          direction(
              [&dir]
              {
                  using math::norm;
                  const auto n = norm(dir);
                  if (n < 1e-8f)
                  {
                      return math::Vector<3>{ 0.0f, -1.0f, 0.0f };
                  }
                  return dir / n;
              }())
    {
    }

    void
    apply() const override;
};

class PointLight : public Light
{
  public:
    math::Vector<3> position;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;

    explicit PointLight(GLenum gl_light_id)
        : Light(gl_light_id),
          position({ 0.0f, 0.0f, 0.0f }),
          constant_attenuation(1.0f),
          linear_attenuation(0.0f),
          quadratic_attenuation(0.0f)
    {
    }

    void
    apply() const override;
};

class SpotLight : public Light
{
  public:
    math::Vector<3> position;
    math::Vector<3> direction;

    float cutoff_angle;
    float exponent;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;

    SpotLight(GLenum gl_light_id, float cutoff = 45.0f)
        : Light(gl_light_id),
          position({ 0.0f, 0.0f, 0.0f }),
          direction({ 0.0f, -1.0f, 0.0f }),
          cutoff_angle(cutoff),
          exponent(0.0f),
          constant_attenuation(1.0f),
          linear_attenuation(0.0f),
          quadratic_attenuation(0.0f)
    {
    }

    void
    apply() const override;
};

}
