#include <GL/gl.h>

namespace ome::open_gl {

class MatrixGuard
{
  public:
    MatrixGuard()
    {
        glGetIntegerv(GL_MATRIX_MODE, &mode_);
        prev_mode_ = mode_;

        glPushMatrix();
    }

    explicit MatrixGuard(GLint mode)
        : mode_(mode)
    {
        glGetIntegerv(GL_MATRIX_MODE, &prev_mode_);

        glMatrixMode(mode_);
        glPushMatrix();
    }

    ~MatrixGuard()
    {
        glMatrixMode(mode_);
        glPopMatrix();
        glMatrixMode(prev_mode_);
    }

    MatrixGuard(const MatrixGuard &) = delete;
    MatrixGuard &
    operator=(const MatrixGuard &)
        = delete;

  private:
    GLint prev_mode_{};
    GLint mode_{};
};

} // namespace ome::open_gl
