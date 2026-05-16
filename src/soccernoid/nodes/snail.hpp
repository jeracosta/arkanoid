#pragma once

#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/open_gl/render_billboard.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class SnailNode : public ome::TransformNode
{
  public:
    explicit SnailNode(ome::Vec3f local_position)
        : TransformNode(local_position + ome::up * size_)
    {
    }

  private:
    static constexpr float size_ = 0.2f;

    void
    on_tick_() override
    {
        auto &camera = game()->camera;

        auto position = transform<ome::Space::World>().position;

        static const auto material
            = ome::Material{ .texture = textures.snail, .blend_mode = ome::BlendMode::alpha() };

        game()->schedule([&]
        { ome::open_gl::render_billboard(position, { size_ }, material, camera); });

        TransformNode::on_tick_();
    }
};

} // namespace soccernoid
