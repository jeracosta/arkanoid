#pragma once

#include "oh-my-engine/light.hpp"
#include "oh-my-engine/nodes/light_node.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

class SceneLightNode : public SoccernoidNode<ome::LightNode<ome::DirectionalLight>>
{
  private:
    using LightSettings = settings::GlobalLight;

    void
    apply_(const LightSettings &light)
    {
        light_.direction      = light.direction;
        light_.color.ambient  = light.ambient;
        light_.color.diffuse  = light.diffuse;
        light_.color.specular = light.specular;
    }

  public:
    void
    on_mount_() override
    {
        apply_(game()->settings.get<LightSettings>());
        hold(game()->settings.bind([this](const LightSettings &light) { apply_(light); }));
    }
};

} // namespace soccernoid
