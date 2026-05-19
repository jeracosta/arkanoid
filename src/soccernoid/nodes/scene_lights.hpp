#pragma once

#include <memory>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/light_node.hpp"

namespace soccernoid {

class SceneLightsNode : public ome::Node
{
  public:
    SceneLightsNode() = default;

    void
    on_mount_() override
    {
        auto sun           = ome::DirectionalLight{};
        sun.direction      = ome::Vec3f{ -0.3f, -1.0f, -0.5f };
        sun.color.ambient  = ome::Color::rgba(0.1f, 0.1f, 0.15f, 1.0f);
        sun.color.diffuse  = ome::Color::rgb(0.9f, 0.9f, 0.85f);
        sun.color.specular = ome::Color::rgb(1.0f, 1.0f, 1.0f);

        auto sun_node = std::make_shared<ome::LightNode>(sun);

        auto fill           = ome::DirectionalLight{};
        fill.direction      = ome::Vec3f{ 0.5f, -0.2f, 0.8f };
        fill.color.ambient  = ome::Color::rgb(0.0f, 0.0f, 0.0f);
        fill.color.diffuse  = ome::Color::rgb(0.3f, 0.35f, 0.4f);
        fill.color.specular = ome::Color::rgb(0.0f, 0.0f, 0.0f);

        auto fill_node = std::make_shared<ome::LightNode>(fill);

        add_child(sun_node);
        add_child(fill_node);
    }
};

} // namespace soccernoid
