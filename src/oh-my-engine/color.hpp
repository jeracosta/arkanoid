#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/ext/vector_float3.hpp>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Color
{
  private:
    Vec4f rgba_;

    template <typename... Args>
    Color(Args... args)
        : rgba_{ args... }
    {
    }

  public:
    Color() = default; // black

    template <typename... Args>
    static constexpr Color
    rgba(Args... args)
    {
        return Color(args...);
    }

    template <typename... Args>
    static constexpr Color
    rgb(Args... args)
    {
        return rgba(args..., 1.0f);
    }

    Vec4f
    rgba() const
    {
        return rgba_;
    }

    float
    red() const
    {
        return rgba_[0];
    }

    float
    green() const
    {
        return rgba_[1];
    }

    float
    blue() const
    {
        return rgba_[2];
    }

    float
    alpha() const
    {
        return rgba_[3];
    }
};

inline Color
grayscale(Color color)
{
    return Color::rgba(dot(color.rgba(), Vec4f(0.299, 0.587, 0.114, 1)));
}

} // namespace ome

inline void
glColor(ome::Color color)
{
    glColor3f(color.red(), color.green(), color.blue());
}
