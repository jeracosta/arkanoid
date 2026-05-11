#include "oh-my-engine/texture.hpp"

#include <GL/glu.h>
#include <SDL2/SDL_image.h>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/open_gl/texture_binding_guard.hpp"

namespace ome {

Texture::Texture(GLuint id, Vec2u size)
    : id_(id),
      size_(size)
{
}

Texture::~Texture()
{
    if (id_ != 0)
    {
        glDeleteTextures(1, &id_);
    }
}

std::shared_ptr<Texture>
Texture::load(const std::filesystem::path &path, Vec2<GLenum> wrap)
{
    SDL_Surface *surface = IMG_Load(path.string().c_str());
    if (!surface)
    {
        throw std::runtime_error(
            std::format("Failed to load texture `{}`: {}", path.string(), IMG_GetError()));
    }

    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);

    SDL_FreeSurface(surface);

    if (!rgba)
    {
        throw std::runtime_error(
            std::format("Failed to convert texture `{}`: {}", path.string(), SDL_GetError()));
    }

    GLuint id = 0;
    glGenTextures(1, &id);

    {
        auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

        glBindTexture(GL_TEXTURE_2D, id);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap[1]);

        gluBuild2DMipmaps(
            GL_TEXTURE_2D, GL_RGBA, rgba->w, rgba->h, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    }

    auto texture = std::shared_ptr<Texture>(new Texture(id, { rgba->w, rgba->h }));

    SDL_FreeSurface(rgba);

    return texture;
}

std::shared_ptr<Texture>
Texture::dummy(Vec2u size)
{
    struct Pixel
    {
        uint8_t red, green, blue, alpha;
    };
    static_assert(std::is_trivially_copyable_v<Pixel>);
    static_assert(sizeof(Pixel) == 4);

    auto &[width, height] = size;

    auto pixels = std::vector<Pixel>(width * height);

    using namespace std::ranges::views;
    auto coords = zip(iota(0u, height), iota(0u, width))
                  | transform([](auto &&pair) { return Vec2u{ pair }; });

    for (auto &&[pixel, coord] : zip(pixels, coords))
    {
        constexpr int checker_size = 8;

        auto cell = coord / checker_size;

        bool is_even = (cell[0] + cell[1]) % 2 == 0;

        pixel = is_even ? Pixel{ 255, 0, 255, 255 } : Pixel{ 0, 0, 0, 255 };
    }

    GLuint id;
    glGenTextures(1, &id);

    {
        auto guard = open_gl::TextureBindingGuard<GL_TEXTURE_2D>();

        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }

    return std::shared_ptr<Texture>(new Texture(id, size));
}

} // namespace ome
