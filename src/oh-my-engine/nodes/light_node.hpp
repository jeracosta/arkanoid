#pragma once

#include <type_traits>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/render_frame.hpp"

namespace ome {

template <std::convertible_to<Light> TLight>
class LightNode : public TransformNode
{
  protected:
    TLight light_;

  private:
    std::size_t light_instances_;

    Vec3f
    direction_()
    {
        auto       dir  = transform<Space::World>().orientation * down;
        const auto norm = math::norm(dir);
        if (norm < 1e-8f)
        {
            return down;
        }
        return dir / norm;
    }

  public:
    explicit LightNode(TLight light = {}, std::size_t instances = 1)
        : light_(std::move(light)),
          light_instances_(instances)
    {
    }

    void
    on_render_(RenderFrame &frame) override
    {
        const auto world = transform<Space::World>();

        if constexpr (std::is_same_v<TLight, PointLight>)
        {
            light_.position = world.position;
        }
        else if constexpr (std::is_same_v<TLight, SpotLight>)
        {
            light_.position  = world.position;
            light_.direction = direction_();
        }
        else if constexpr (std::is_same_v<TLight, DirectionalLight>)
        {
            light_.direction = world.orientation * light_.direction;
            const auto norm  = math::norm(light_.direction);
            if (norm > 1e-8f)
            {
                light_.direction /= norm;
            }
        }
        else
        {
            static_assert(!"Unsupported light type");
        }

        for (auto _ : std::views::iota(0u, light_instances_))
        {
            frame.lights.push_back(light_);
        }
    }

    const Light &
    light() const
    {
        return light_;
    }
};

} // namespace ome
