#include <GL/gl.h>
namespace ome::open_gl {

// RAII guard for OpenGL matrix mode.
// Saves the current matrix mode on construction and restores it on destruction.
struct MatrixModeGuard
{
  private:
    GLint prev_;

  public:
    MatrixModeGuard()
    {
        glGetIntegerv(GL_MATRIX_MODE, &prev_);
    }

    ~MatrixModeGuard()
    {
        glMatrixMode(prev_);
    }
};

} // namespace ome::open_gl
