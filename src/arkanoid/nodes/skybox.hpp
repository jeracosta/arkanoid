#pragma once

#include "oh-my-engine/render_frame.hpp"
#include "oh-my-engine/skybox.hpp"
#include "arkanoid/constants.hpp"
#include "arkanoid/nodes/arkanoid_node.hpp"
#include "arkanoid/settings.hpp"

namespace arkanoid {

class SkyboxNode : public ArkanoidNode<>
{
  public:
    // Default: follow the skybox chosen in the settings menu.
    SkyboxNode() = default;

    // Explicit: pin a specific skybox (e.g. story screens), ignoring the setting.
    explicit SkyboxNode(TexturePalette::SkyboxFaces skybox)
        : skybox_(std::move(skybox)),
          follow_setting_(false)
    {
    }

    void
    on_render_(ome::RenderFrame &frame) override
    {
        const auto &skybox = active_skybox_();

        frame.skybox = ome::Skybox
        {
            .size     = game()->camera.far(),
            .textures = {
                .front  = skybox.front,
                .back   = skybox.back,
                .left   = skybox.left,
                .right  = skybox.right,
                .top    = skybox.top,
                .bottom = skybox.bottom,
            },
        };
    }

  private:
    const TexturePalette::SkyboxFaces &
    active_skybox_() const
    {
        if (!follow_setting_ || textures.skybox.empty())
        {
            return skybox_;
        }

        int index = game()->settings.get<settings::render::Skybox>().value;
        if (index < 0 || index >= static_cast<int>(textures.skybox.size()))
        {
            index = 0;
        }

        return textures.skybox[index];
    }

    TexturePalette::SkyboxFaces skybox_         = textures.skybox.front();
    bool                        follow_setting_ = true;
};

} // namespace arkanoid
