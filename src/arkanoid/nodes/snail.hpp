#pragma once

#include "oh-my-engine/draw_command.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "arkanoid/constants.hpp"

namespace arkanoid {

class SnailNode : public ome::TransformNode
{
  private:
    static constexpr float size_ = 0.2f;

    void
    on_render_(ome::RenderFrame &frame) override
    {
        auto position = transform<ome::Space::World>().position + ome::up * size_ / 2;

        auto material = ome::Material{
            .texture    = textures.snail,
            .blend_mode = ome::BlendMode::alpha(),
        };

        frame.draw_commands.push_back(
            ome::DrawCommand::billboard(position, { size_ }, material, game()->camera));
    }
};

} // namespace arkanoid
