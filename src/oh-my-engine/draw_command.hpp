#pragma once

#include <memory>

#include "oh-my-engine/material.hpp"
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
};

} // namespace ome
