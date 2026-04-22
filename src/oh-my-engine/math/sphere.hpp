#pragma once

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

template <is_vector Vector>
class Sphere
{
  private:
    Vector center_;
    float  radius_;

  public:
    Sphere() = default;

    Sphere(const Vector &center, float radius)
        : center_(center),
          radius_(radius)
    {
    }

    const Vector &
    center() const
    {
        return center_;
    }

    float
    radius() const
    {
        return radius_;
    }

    bool
    contains(const Vector &point) const
    {
        return length(point - center_) <= radius_;
    }
};

} // namespace ome::math
