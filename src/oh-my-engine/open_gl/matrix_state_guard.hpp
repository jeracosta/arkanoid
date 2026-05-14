#include <GL/gl.h>
namespace ome::open_gl {

// RAII guard for OpenGL matrix state.
// Saves the current matrix state on construction and restores it on destruction.
struct MatrixStateGuard
{
  public:
    MatrixStateGuard()
    {
        glPushMatrix();
    }

    ~MatrixStateGuard()
    {
        glPopMatrix();
    }
};

} // namespace ome::open_gl
