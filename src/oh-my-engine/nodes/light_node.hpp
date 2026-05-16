#pragma once

#include <memory>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

namespace light_node_detail {

inline Vec3f
unit_light_direction_from_transform(const TransformNode::Component &transform)
{
    auto v = transform.orientation * down;
    const auto n = math::norm(v);
    if (n < 1e-8f)
    {
        return down;
    }
    return v / n;
}

}

class LightNode : public TransformNode
{
  private:
    std::unique_ptr<Light> light_;

  public:
    explicit LightNode(std::unique_ptr<Light> light)
        : light_(std::move(light))
    {
    }

    void
    on_tick_() override
    {
        auto transform = world_transform();

        if (auto *point = dynamic_cast<PointLight *>(light_.get()))
        {
            point->position = transform.position;
        }
        else if (auto *spot = dynamic_cast<SpotLight *>(light_.get()))
        {
            spot->position  = transform.position;
            spot->direction = light_node_detail::unit_light_direction_from_transform(transform);
        }
        else if (auto *dir = dynamic_cast<DirectionalLight *>(light_.get()))
        {
            dir->direction = light_node_detail::unit_light_direction_from_transform(transform);
        }

        light_->apply();
    }

    Light *
    light()
    {
        return light_.get();
    }

    const Light *
    light() const
    {
        return light_.get();
    }
};

}
