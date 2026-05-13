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

    Box(Component side_length)
        : min_(Vector(-side_length / Component(2))),
          max_(Vector(side_length / Component(2)))
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

    Vector
    anchor() const
    {
        return (min_ + max_) / Component(2);
    }

    void
    displace(const Vector &delta)
    {
        min_ += delta;
        max_ += delta;
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

    // clang-format off

    friend bool
    overlaps(const Box &a, const Box &b)
    {
        constexpr auto compare = [](auto &a, auto &b)
        {
            return component_wise(std::less{}, a.min, b.max);
        };

        return compare(a, b) && compare(b, a);
    }

    // clang-format on

    auto
    corners() const
    {
        auto count = std::size_t{ 1 } << Dimension;

        auto indices = std::views::iota(std::size_t{ 0 }, count);

        auto make_corner = [this](std::size_t indice)
        {
            Vector corner{};
            for (std::size_t j = 0; j < Dimension; ++j)
            {
                corner[j] = (indice & (std::size_t{ 1 } << j)) ? max_[j] : min_[j];
            }
            return corner;
        };

        return indices | std::views::transform(make_corner);
    }

        return result;
    }

  private:
    Vector min_{};
    Vector max_{};
};

} // namespace ome::math
