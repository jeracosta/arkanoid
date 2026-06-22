#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "oh-my-engine/texture.hpp"
#include "arkanoid/constants.hpp"
#include "arkanoid/events.hpp"
#include "arkanoid/nodes/arkanoid_node.hpp"

namespace arkanoid {

class GoalNode : public ArkanoidNode<ome::HitboxNode>
{
  public:
    struct Configuration
    {
        ome::Vec3f position;
        ome::Vec3f size;
        ome::Color color;

        static Configuration
        defaults()
        {
            return {
                .position = { 0.0f, 1.5f, -4.5f },
                .size     = { 2.0f, 2.0f, 0.3f },
                .color    = ome::Color::rgb(230, 242, 217),
            };
        }
    };

  private:
    Configuration config_;
    bool          hit_ = false;

    void
    render_()
    {
        auto region = hitbox<ome::Space::World>();
        auto color  = hit_ ? ome::Color::rgb(100, 255, 100) : config_.color;

        glColor(color);

        auto placeholder = ome::Texture::placeholder();

        using Bounds = ome::Rect::Bounds;
        ome::Sprite sprite{ placeholder, Bounds{ { 0.0f, 0.0f }, { 1.0f, 1.0f } } };

        ome::open_gl::BoxRenderTask{
            .world_region = region,
            .sprites      = { sprite, sprite, sprite, sprite, sprite, sprite },
        }();

        glColor(ome::Color::white());
    }

  public:
    explicit GoalNode(const Configuration &config)
        : ArkanoidNode<ome::HitboxNode>(config.size),
          config_(config)
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = config_.position; });
    }

    void
    on_collision_(ome::HitboxNode &other) override
    {
        if (hit_)
        {
            return;
        }

        if (other.name().find("Hitbox") == std::string::npos)
        {
            return;
        }

        hit_ = true;
        log("GOAL! Player scored!");
        game()->Events.emit(GoalHit{});
    }

    void
    on_tick_() override
    {
        render_();
    }
};

} // namespace arkanoid
