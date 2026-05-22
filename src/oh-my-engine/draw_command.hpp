#pragma once

#include <memory>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/transform.hpp"

namespace ome {

struct DrawCommand
{
    enum class Layer
    {
        Opaque,
        Transparent,
        Overlay,
    };

    std::shared_ptr<Mesh> mesh;
    std::vector<Material> materials;
    Transform             transform;
    Layer                 layer = Layer::Opaque;

    static DrawCommand
    box(const Box                  &box,
        std::vector<Material>       materials,
        const BoxFaces<std::size_t> material_indices,
        std::size_t                 subdivisions = 1);

    static DrawCommand
    quad(Material material);

    static DrawCommand
    billboard(Vec3f position, Vec2f size, const Material &material, const Camera &camera);

    static DrawCommand
    pyramid(Vec3f apex, Vec3f direction, float height, Vec2f base_half_extents,
            std::vector<Material> materials);
};

} // namespace ome
