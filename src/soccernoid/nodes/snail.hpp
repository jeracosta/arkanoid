#pragma once

#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/open_gl/render_billboard.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class SnailNode : public ome::TransformNode
{
  private:
    static constexpr float size_ = 0.2f;

    void
    on_tick_() override
    {
        auto &camera = game()->camera;

        auto position = transform<ome::Space::World>().position + ome::up * size_ / 2;

        static const ome::Material material = []
        {
            ome::Material m;
            m.texture    = textures.snail;
            m.blend_mode = ome::BlendMode::alpha();
            return m;
        }();

        game()->schedule([position, &camera]
        { ome::open_gl::render_billboard(position, { size_ }, material, camera); });

        TransformNode::on_tick_();
    }
};

} // namespace soccernoid
