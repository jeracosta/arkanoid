#pragma once

#include <memory>
#include <optional>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

class MeshNode : public TransformNode
{
  public:
    MeshNode(std::shared_ptr<Mesh> mesh,
             std::optional<Sprite> sprite   = std::nullopt,
             std::optional<Color>  modulate = std::nullopt)
        : mesh_(std::move(mesh)),
          sprite_(std::move(sprite)),
          modulate_(std::move(modulate))
    {
    }

    void
    set_sprite(std::optional<Sprite> sprite)
    {
        sprite_ = std::move(sprite);
    }

    void
    set_modulate(std::optional<Color> modulate)
    {
        modulate_ = std::move(modulate);
    }

    const std::shared_ptr<Mesh> &
    mesh() const noexcept
    {
        return mesh_;
    }

    void
    on_tick_() override
    {
        if (!mesh_)
        {
            return;
        }

        // TODO: render mesh
    }

  private:
    std::shared_ptr<Mesh>     mesh_;
    std::optional<Sprite>     sprite_;
    std::optional<ome::Color> modulate_;
};

} // namespace ome
