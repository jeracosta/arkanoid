#pragma once

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

template <std::size_t Dimension, typename Component = float>
class Box
{
  public:
    using Vector        = Vector<Dimension, Component>;
    using ComponentType = Component;

    static constexpr std::size_t
    dimension()
    {
        return Dimension;
    }

    Box() = default;

    Box(Vector min, Vector max)
        : min_(std::move(min)),
          max_(std::move(max))
    {
    }

    const Vector &
    min() const
    {
        return min_;
    }

    const Vector &
    max() const
    {
        return max_;
    }

    bool
    contains(const Vector &point) const
    {
        for (std::size_t i = 0; i < Dimension; ++i)
        {
            if (point[i] < min_[i] || point[i] > max_[i])
            {
                return false;
            }
        }
        return true;
    }

    template <class Rng>
    Vector
    sample_uniform(Rng &rng) const
    {
        Vector result;
        for (std::size_t i = 0; i < Dimension; ++i)
        {
            std::uniform_real_distribution<Component> dist(min_[i], max_[i]);
            result[i] = dist(rng);
        }
        return result;
    }

  private:
    Vector min_{};
    Vector max_{};
};

} // namespace ome::math
