#pragma once

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/open_gl/render_quad.hpp"

namespace ome::open_gl {

inline void
render_billboard(Vec3f position, Vec2f size, const Material &material, const Camera &camera)
{
    Vec3f right = camera.right() * (size[0] * 0.5f);
    Vec3f up    = camera.up() * (size[1] * 0.5f);

    std::array<Vec3f, 4> vertices = {
        position - right - up,
        position + right - up,
        position + right + up,
        position - right + up,
    };

    render_quad(vertices, material);
}

} // namespace ome::open_gl
