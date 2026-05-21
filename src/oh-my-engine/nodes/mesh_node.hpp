#pragma once

#include <memory>

#include "oh-my-engine/material.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

class MeshNode : public TransformNode
{
  public:
    MeshNode(std::shared_ptr<Mesh> mesh, Material material)

        : mesh_(std::move(mesh)),
          material_(std::move(material))
    {
    }

    void
    on_render_(RenderFrame &frame) override
    {
        if (mesh_ == nullptr)
        {
            return;
        }

        frame.draw_commands.push_back(DrawCommand{
            .mesh      = mesh_,
            .materials = { material_ },
            .transform = transform<Space::World>(),
        });
    }

  private:
    std::shared_ptr<Mesh> mesh_;
    Material              material_;
};
} // namespace ome
