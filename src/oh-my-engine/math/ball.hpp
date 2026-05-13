#pragma once

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

template <std::size_t N, typename Component = float>
class Ball
{
  public:
    using Vector        = Vector<N, Component>;
    using ComponentType = Component;
    static constexpr std::size_t
    dimension()
    {
        return N;
    }

    Ball() = default;

    Ball(Vector center, Component radius)
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
        return math::norm(point - center_) <= radius_;
    }

    template <class Rng>
    Vector
    sample_uniform(Rng &rng) const
    {
        auto uniform = std::uniform_real_distribution<Component>{ 0, 1 }(rng);
        auto r       = radius_ * std::pow(uniform, 1 / N);
        return center_ + random_unit_vector<Vector>(rng) * r;
    }

  private:
    Vector    center_{};
    Component radius_ = Component(1);
};

} // namespace ome::math
