#pragma once

#include <type_traits>
#include <variant>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

#include <GL/gl.h>

namespace ome::open_gl {
void bind(const Light &light, GLenum slot);
}

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
    Light light_;

  public:
    explicit LightNode(Light light)
        : light_(std::move(light))
    {
    }

    explicit LightNode(const PointLight &light)
        : light_(light)
    {
    }

    explicit LightNode(const SpotLight &light)
        : light_(light)
    {
    }

    explicit LightNode(const DirectionalLight &light)
        : light_(light)
    {
    }

    void
    on_tick_() override
    {
        const auto world = transform<Space::World>();

        // Light is a std::variant; update fields via std::visit.
        std::visit(
            [&](auto &light)
            {
                using T = std::remove_cvref_t<decltype(light)>;
                if constexpr (std::is_same_v<T, PointLight>)
                {
                    light.position = world.position;
                }
                else if constexpr (std::is_same_v<T, SpotLight>)
                {
                    light.position  = world.position;
                    light.direction = light_node_detail::unit_light_direction_from_transform(world);
                }
                else if constexpr (std::is_same_v<T, DirectionalLight>)
                {
                    light.direction = light_node_detail::unit_light_direction_from_transform(world);
                }
            },
            light_);

        // Bind to the first slot every tick.
        ome::open_gl::bind(light_, GL_LIGHT0);
        glEnable(GL_LIGHT0);
    }

    Light &
    light()
    {
        return light_;
    }

    const Light &
    light() const
    {
        return light_;
    }
};

}
