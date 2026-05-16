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
        auto sun = std::make_unique<ome::DirectionalLight>(GL_LIGHT0, ome::Vec3f{ -0.3f, -1.0f, -0.5f });
        sun->ambient  = ome::Color::rgba(0.1f, 0.1f, 0.15f, 1.0f);
        sun->diffuse  = ome::Color::rgb(0.9f, 0.9f, 0.85f);
        sun->specular = ome::Color::rgb(1.0f, 1.0f, 1.0f);

        auto sun_node = std::make_shared<ome::LightNode>(std::move(sun));

        auto fill = std::make_unique<ome::DirectionalLight>(GL_LIGHT1, ome::Vec3f{ 0.5f, -0.2f, 0.8f });
        fill->ambient  = ome::Color::rgb(0.0f, 0.0f, 0.0f);
        fill->diffuse  = ome::Color::rgb(0.3f, 0.35f, 0.4f);
        fill->specular = ome::Color::rgb(0.0f, 0.0f, 0.0f);

        auto fill_node = std::make_shared<ome::LightNode>(std::move(fill));

        add_child(sun_node);
        add_child(fill_node);
    }
};

}
