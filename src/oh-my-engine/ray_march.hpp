// https://en.wikipedia.org/wiki/Ray_marching
// https://en.wikipedia.org/wiki/Signed_distance_function

#pragma once

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <optional>

namespace ome::ray_march {

class Cube
{
  private:
    glm::vec3 half_extent_;

  public:
    Cube(float side_length)
        : half_extent_(side_length / 2)
    {
    }

    float
    distance(glm::vec3 p) const
    {
        glm::vec3 q = abs(p) - glm::vec3(half_extent_);
        return length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
    }
};

class Sphere
{
    float radius_;

  public:
    Sphere(float radius)
        : radius_(radius)
    {
    }

    float
    distance(glm::vec3 p) const
    {
        return length(p) - radius_;
    }
};

// Consider the space as a tesselation of cubes of `cell_size` side length, this function maps any
// point `p` to the corresponding point in the cube centered at the origin.
inline glm::vec3
repeat_centered(glm::vec3 p, float cell_size)
{
    return mod(p + cell_size / 2, cell_size) - cell_size / 2;
}

struct MengerSponge
{
    int       levels          = 4;                // fractal iterations
    float     iteration_scale = 2.7f;             // scale applied each fold
    glm::vec3 fold_offset     = glm::vec3(-1.0f); // translation after scaling

    float
    distance(glm::vec3 p) const
    {
        float de_scale = 1.0f;

        for (int i = 0; i < levels; ++i)
        {
            p = abs(p);

            if (p.x < p.y)
                std::swap(p.x, p.y);
            if (p.x < p.z)
                std::swap(p.x, p.z);
            if (p.y < p.z)
                std::swap(p.y, p.z);

            p = p * iteration_scale + fold_offset;

            if (p.z > 1.0f)
                p.z -= 2.0f;

            de_scale *= iteration_scale;
        }

        return Cube(1.0f).distance(p) / de_scale;
    }
};

glm::vec3
twist_y(const glm::vec3 &p, float angle, float plane_y = 0.0f)
{
    float dy    = p.y - plane_y;
    float scale = 1.0f / (1.0f + std::abs(dy));
    float theta = angle * dy * scale; // opposite rotation above/below plane

    float c = std::cos(theta);
    float s = std::sin(theta);

    return glm::vec3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}

struct RayMarchResult
{
    struct Hit
    {
        glm::vec3 point;
        glm::vec3 normal;
    };

    std::optional<Hit> hit;
    float              closest_distance;
};

RayMarchResult
ray_march(glm::vec3 origin,
          glm::vec3 direction,
          auto    &&field, // callable like `float(*)(glm::vec3)`
          int       max_steps,
          float     max_ray_length,
          float     epsilon)
{
    float ray_length   = 0.0f;
    float min_distance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < max_steps; i++)
    {
        glm::vec3 p  = origin + direction * ray_length;
        float     d  = field(p);
        min_distance = glm::min(min_distance, d);

        if (d < epsilon)
        {
            const float h = 1e-4f;
            glm::vec3   n = glm::normalize(
                glm::vec3(field(p + glm::vec3(h, 0, 0)) - field(p - glm::vec3(h, 0, 0)),
                          field(p + glm::vec3(0, h, 0)) - field(p - glm::vec3(0, h, 0)),
                          field(p + glm::vec3(0, 0, h)) - field(p - glm::vec3(0, 0, h))));
            return { RayMarchResult::Hit{ p, n }, ray_length };
        }

        ray_length += d;
        if (ray_length > max_ray_length)
            break;
    }

    return { std::nullopt, min_distance };
}

class Pyramid
{
    glm::vec3 center_; // centroid of the whole pyramid
    glm::vec3 dir_;    // normalized base -> apex
    glm::mat3 basis_;

    float height_;
    float base_side_;
    float inv_scale_;
    float h_;

    static inline float
    sdPyramid(glm::vec3 p, float h)
    {
        const float m2 = h * h + 0.25f;

        glm::vec2 xz = glm::abs(glm::vec2(p.x, p.z));
        if (xz.y > xz.x)
            std::swap(xz.x, xz.y);
        xz -= 0.5f;

        p.x = xz.x;
        p.z = xz.y;

        glm::vec3 q(p.z, h * p.y - 0.5f * p.x, h * p.x + 0.5f * p.y);

        float s = glm::max(-q.x, 0.0f);
        float t = glm::clamp((q.y - 0.5f * p.z) / (m2 + 0.25f), 0.0f, 1.0f);

        float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
        float b = m2 * (q.x + 0.5f * t) * (q.x + 0.5f * t) + (q.y - m2 * t) * (q.y - m2 * t);

        float d2 = (glm::min(q.y, -q.x * m2 - 0.5f * q.y) > 0.0f) ? 0.0f : glm::min(a, b);

        return std::sqrt((d2 + q.z * q.z) / m2) * glm::sign(glm::max(q.z, -p.y));
    }

  public:
    Pyramid(float     height,
            float     base_side,
            glm::vec3 center    = glm::vec3(0),
            glm::vec3 direction = glm::vec3(0, 1, 0))
        : center_(center),
          dir_(glm::normalize(direction)),
          height_(height),
          base_side_(base_side),
          inv_scale_(1.0f / base_side),
          h_(height / base_side)
    {
        glm::vec3 up    = dir_;
        glm::vec3 right = glm::normalize(
            glm::cross(std::abs(up.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0), up));
        glm::vec3 forward = glm::cross(up, right);
        basis_            = glm::transpose(glm::mat3(right, up, forward));
    }

    float
    distance(glm::vec3 p) const
    {
        glm::vec3 base_center = center_ - dir_ * (height_ * 0.25f);
        p                     = basis_ * (p - base_center);
        p *= inv_scale_;
        return sdPyramid(p, h_) * base_side_;
    }
};

} // namespace ome::ray_march
