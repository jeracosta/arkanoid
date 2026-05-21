#pragma once

#include <type_traits>
#include <variant>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/render_frame.hpp"

namespace ome {

class LightNode : public TransformNode
{
  private:
    Light light_;

    Vec3f
    direction_()
    {
        auto       v    = transform<Space::World>().orientation * down;
        const auto norm = math::norm(v);
        if (norm < 1e-8f)
        {
            return down;
        }
        return v / norm;
    }

  public:
    explicit LightNode(Light light)
        : light_(std::move(light))
    {
    }

    void
    on_render_(RenderFrame &frame) override
    {
        const auto world = transform<Space::World>();

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
                light.direction = direction_();
            }
            else if constexpr (std::is_same_v<T, DirectionalLight>)
            {
                light.direction = world.orientation * light.direction;
                const auto norm = math::norm(light.direction);
                if (norm > 1e-8f)
                {
                    light.direction /= norm;
                }
            }
        },
            light_);

        frame.lights.push_back(light_);
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

} // namespace ome
