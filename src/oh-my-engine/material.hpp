#pragma once

#include <GL/gl.h>
#include <memory>

#include "oh-my-engine/blend_mode.hpp"
#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

struct Material
{
    struct
    {
        Color ambient  = Color::hex(0x333333FF);
        Color diffuse  = Color::hex(0xCCCCCCFF);
        Color specular = Color::hex(0x000000FF);
        Color emission = Color::hex(0x000000FF);
        Color base     = Color::hex(0xFFFFFFFF);
    } color;

    float shininess = 0.0f;

    std::shared_ptr<Texture> texture    = nullptr;
    TextureEnvironmentMode   env_mode   = TextureEnvironmentMode::Modulate;
    BlendMode                blend_mode = BlendMode::opaque();

    Material() = default;
};

} // namespace ome

namespace ome::open_gl {

void
bind(const Material &material);

}
