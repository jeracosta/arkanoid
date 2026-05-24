#include "oh-my-engine/texture.hpp"

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstring>
#include <ranges>
#include <stdexcept>

#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/open_gl/texture_binding_guard.hpp"

namespace ome {

Texture::Texture(Primitive image)
{
    auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

    glGenTextures(1, &id_);

    glBindTexture(GL_TEXTURE_2D, id_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    auto &[width, height] = image.size();

    gluBuild2DMipmaps(
        GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.raw());
}

Texture::~Texture()
{
    glDeleteTextures(1, &id_);
}

std::shared_ptr<Texture>
Texture::load(const std::filesystem::path &path)
{
    SDL_Surface *raw_surface = IMG_Load(path.string().c_str());
    if (!raw_surface)
    {
        throw std::runtime_error(
            std::format("Failed to load texture `{}`: {}", path.string(), IMG_GetError()));
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(raw_surface, SDL_PIXELFORMAT_RGBA32, 0);

    SDL_FreeSurface(raw_surface);

    if (!surface)
    {
        throw std::runtime_error(
            std::format("Failed to convert texture `{}`: {}", path.string(), SDL_GetError()));
    }

    auto size  = Vec2u{ surface->w, surface->h };
    auto image = Primitive(size);

    std::memcpy(image.raw(), surface->pixels, size[0] * size[1] * sizeof(Primitive::Pixel));

    SDL_FreeSurface(surface);

    return std::make_shared<Texture>(std::move(image));
}

Texture::Primitive
Texture::Primitive::checkerboard(Vec2u size, float cell_size, Color odd_color, Color even_color)
{
    auto image = Primitive(size);

    auto &[width, height] = size;

    using namespace std::ranges::views;
    auto coords = cartesian_product(iota(0u, height), iota(0u, width));

    for (auto &&[x, y] : coords)
    {
        auto cell = Vec2u{ x, y } / cell_size;

        bool is_even = (cell[0] + cell[1]) % 2 == 0;

        image[{ x, y }] = is_even ? even_color : odd_color;
    }

    return image;
}

std::shared_ptr<Texture>
Texture::placeholder()
{
    // We use a magenta checkerboard pattern, inspired by Source engine.

    static constexpr float cell_size = 8;
    static constexpr auto  size      = Vec2u{ 2 } * cell_size;

    static constexpr auto odd_color  = Color::magenta();
    static constexpr auto even_color = Color::black();

    auto        image   = Primitive::checkerboard(size, cell_size, odd_color, even_color);
    static auto texture = std::make_shared<Texture>(image);

    return texture;
}

void
Texture::set_wrap(Vec2<GLenum> wrap)
{
    auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

    glBindTexture(GL_TEXTURE_2D, id_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap[1]);
}

void
Texture::set_filters(GLenum min_filter, GLenum mag_filter)
{
    auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

    glBindTexture(GL_TEXTURE_2D, id_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

} // namespace ome

void
Texture::set_blend_mode(GLenum mode)
{
    auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

    glBindTexture(GL_TEXTURE_2D, id_);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
}

} // namespace ome
