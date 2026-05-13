#pragma once

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

// (N-1)-sphere surface embedded in N-D space.
template <std::size_t Dimension, typename Component = float>
class Sphere
{
  public:
    using Vector        = Vector<Dimension, Component>;
    using ComponentType = Component;
    static constexpr std::size_t
    dimension()
    {
        return Dimension;
    }

    Sphere() = default;

    Sphere(Vector center, Component radius)
        : center_(std::move(center)),
          radius_(radius)
    {
    }

    const Vector &
    center() const
    {
        return center_;
    }

    Component
    radius() const
    {
        return radius_;
    }

    bool
    contains(const Vector &point) const
    {
        auto dist = math::norm(point - center_);
        return std::abs(dist - radius_) <= Component(1.0e-6);
    }

    template <class Rng>
    Vector
    sample_uniform(Rng &rng) const
    {
        return center_ + random_unit_vector<Vector>(rng) * radius_;
    }

  private:
    Vector    center_{};
    Component radius_ = Component(1);
};

} // namespace ome::math
