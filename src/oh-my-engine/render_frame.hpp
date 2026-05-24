#pragma once

#include "oh-my-engine/draw_command.hpp"
#include "oh-my-engine/fog.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/skybox.hpp"

namespace ome {

struct RenderFrame
{
    // Some aspects of rendering were not migrated to the new render frame approach yet.
    // TODO: add .camera
    // TODO: add .fog

    std::optional<Skybox> skybox;

    std::vector<Light> lights;

    std::vector<DrawCommand> draw_commands;

    // When true, scene geometry is drawn as wireframe (skybox and overlays stay solid).
    bool wireframe = false;

    void
    clear()
    {
        *this = {};
    }
};

} // namespace ome

namespace ome::open_gl {

void
render(const RenderFrame &frame);

}
