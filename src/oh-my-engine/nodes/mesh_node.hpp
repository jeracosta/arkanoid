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
          materials_(std::vector{ std::move(material) })
    {
    }

    MeshNode(std::shared_ptr<Mesh> mesh, std::vector<Material> materials)

        : mesh_(std::move(mesh)),
          materials_(std::move(materials))
    {
    }

    template <typename F>
    void
    update_mesh(const F &&function)
    {
        function(mesh_);
    }

    template <typename F>
    void
    update_material(const F &&function)
    {
        function(materials_.front());
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
            .materials = materials_,
            .transform = transform<Space::World>(),
        });
    }

  private:
    std::shared_ptr<Mesh> mesh_;
    std::vector<Material> materials_;
};
} // namespace ome
