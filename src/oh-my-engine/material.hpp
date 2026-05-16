#pragma once

#include <GL/gl.h>

#include "oh-my-engine/blend_mode.hpp"
#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

struct Material
{
    std::shared_ptr<Texture> texture    = nullptr;
    Color                    color      = Color::white();
    BlendMode                blend_mode = BlendMode::opaque();
};

} // namespace ome
