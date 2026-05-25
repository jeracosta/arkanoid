#pragma once

#include "oh-my-engine/draw_command.hpp"
#include "oh-my-engine/fog.hpp"
#include "oh-my-engine/light.hpp"
#include "oh-my-engine/math/interval.hpp"
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

    // When false, scene geometry is drawn untextured (flat material colors).
    bool textures_enabled = true;

    // true: smooth (GL_SMOOTH) per-vertex shading · false: flat (GL_FLAT) per-face shading.
    bool smooth_shading = true;

    // Debug overlay: world-space hitbox boxes, drawn as wireframe only when show_hitboxes is set.
    bool             show_hitboxes = false;
    std::vector<Box> hitboxes;

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
