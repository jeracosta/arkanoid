#include <GL/gl.h>

namespace ome::open_gl {

// RAII guard for texture binding.
// Saves the currently bound texture and active texture unit on construction, and restores them on
// destruction.
template <GLenum TTarget = GL_TEXTURE_2D>
struct TextureBindingGuard
{
  private:
    GLint prev_unit_ = 0;
    GLint prev_tex_  = 0;

  public:
    TextureBindingGuard()
    {
        glGetIntegerv(GL_ACTIVE_TEXTURE, &prev_unit_);
        glGetIntegerv(binding_enum(), &prev_tex_);
    }

    ~TextureBindingGuard()
    {
        glActiveTexture(prev_unit_);
        glBindTexture(TTarget, static_cast<GLuint>(prev_tex_));
    }

    static constexpr GLenum
    binding_enum()
    {
        if constexpr (TTarget == GL_TEXTURE_2D)
        {
            return GL_TEXTURE_BINDING_2D;
        }
        else if constexpr (TTarget == GL_TEXTURE_3D)
        {
            return GL_TEXTURE_BINDING_3D;
        }
        else if constexpr (TTarget == GL_TEXTURE_CUBE_MAP)
        {
            return GL_TEXTURE_BINDING_CUBE_MAP;
        }
        else
        {
            static_assert(!"Unsupported texture target");
        }
    }
};

} // namespace ome::open_gl
