#pragma once

#include <GL/gl.h>
#include <filesystem>
#include <memory>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Texture
{
  public:
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

    static std::shared_ptr<Texture>
    load(const std::filesystem::path &path, Vec2<GLenum> wrap = { GL_REPEAT, GL_REPEAT });

    static std::shared_ptr<Texture>
    dummy(Vec2u size);

    GLuint
    id() const
    {
        return id_;
    }

    const Vec2u &
    size() const
    {
        return size_;
    }

  protected:
    GLuint id_   = 0;
    Vec2u  size_ = 0;

    Texture(GLuint id, Vec2u size);
};

namespace open_gl {
inline void
glBindTexture(const ome::Texture &texture)
{
    ::glBindTexture(GL_TEXTURE_2D, texture.id());
}
} // namespace open_gl

} // namespace ome
