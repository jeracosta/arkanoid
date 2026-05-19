#pragma once

#include <GL/gl.h>
#include <filesystem>
#include <memory>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Texture;

struct Sprite
{
    std::shared_ptr<Texture> texture;
    Rect                     uv_region = Rect::from_bounds({ 0.0f, 0.0f }, { 1.0f, 1.0f });
};

class Texture
{
  public:
    class Primitive
    {
      public:
        using Pixel = Color;
        static_assert(std::is_trivially_copyable_v<Pixel>);
        static_assert(sizeof(Pixel) == 4);

        Primitive(Vec2u size)
            : size_(size),
              pixels_(size[0] * size[1])
        {
        }

        const Vec2u
        size()
        {
            return size_;
        }

        Pixel &
        operator[](Vec2u coord)
        {
            return pixels_[coord[1] * size_[0] + coord[0]];
        }

        static Primitive
        load(std::filesystem::path path, Vec2u size);

        static Primitive
        checkerboard(Vec2u size, float cell_size, Color odd_color, Color even_color);

        std::byte *
        raw()
        {
            return reinterpret_cast<std::byte *>(pixels_.data());
        }

      private:
        Vec2u              size_;
        std::vector<Pixel> pixels_; // row-major order
    };

    Texture() = delete;
    ~Texture();

    Texture(const Texture &) = delete;

    Texture &
    operator=(const Texture &)
        = delete;

    Texture(Texture &&other) noexcept = delete;

    Texture &
    operator=(Texture &&other) noexcept
        = delete;

    Texture(Primitive image);

    GLuint
    id()
    {
        return id_;
    }

    static std::shared_ptr<Texture>
    load(const std::filesystem::path &path);

    void
    set_wrap(Vec2<GLenum> wrap = { GL_REPEAT, GL_REPEAT });

    void
    set_filters(GLenum min_filter = GL_LINEAR, GLenum mag_filter = GL_LINEAR);

    void
    set_blend_mode(GLenum mode);

    static std::shared_ptr<Texture>
    placeholder();

  private:
    GLuint id_ = 0;
};

enum class TextureEnvironmentMode
{
    Modulate = GL_MODULATE,
    Decal    = GL_DECAL,
    Blend    = GL_BLEND,
    Replace  = GL_REPLACE,
};

} // namespace ome
