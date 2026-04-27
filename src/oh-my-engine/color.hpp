#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/ext/vector_float3.hpp>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Color
{
  private:
    Vec3f rgb_;

    template <typename... Args>
    Color(Args... args)
        : rgb_{ args... }
    {
    }

  public:
    Color() = default; // black

    template <typename... Args>
    static constexpr Color
    rgb(Args... args)
    {
        return Color(args...);
    }

    Vec3f
    rgb() const
    {
        return rgb_;
    }

    float
    red() const
    {
        return rgb_[0];
    }

    float
    green() const
    {
        return rgb_[1];
    }

    float
    blue() const
    {
        return rgb_[2];
    }
};

inline Color
grayscale(Color color)
{
    return Color::rgb(dot(color.rgb(), Vec3f(0.299, 0.587, 0.114)));
}

} // namespace ome

inline void
glColor(ome::Color color)
{
    glColor3f(color.red(), color.green(), color.blue());
}
