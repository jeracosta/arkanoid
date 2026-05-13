#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <type_traits>
#include <variant>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

// #region Primary template (arithmetic types)

template <typename T>
class UniformDistribution
{
    static_assert(std::is_arithmetic_v<T>,
                  "UniformDistribution<T> requires an arithmetic type. "
                  "Use UniformDistribution<ome::math::Vector<...>> for vectors.");

  public:
    struct Range_
    {
        T min = T(0);
        T max = T(1);

        template <class Rng>
        T
        operator()(Rng &rng) const
        {
            if constexpr (std::is_floating_point_v<T>)
            {
                return std::uniform_real_distribution<T>{ min, max }(rng);
            }
            else
            {
                return std::uniform_int_distribution<T>{ min, max }(rng);
            }
        }
    };

    using Variant_ = std::variant<Range_>;

  private:
    Variant_ distribution_ = Range_{};

  public:
    UniformDistribution() = default;

    template <class Strategy>
    UniformDistribution(Strategy strategy)
        : distribution_(std::move(strategy))
    {
    }

    static UniformDistribution
    range(T min, T max)
    {
        return Range_{ .min = min, .max = max };
    }

    template <class Rng>
    T
    operator()(Rng &rng) const
    {
        return std::visit([&](const auto &d) { return d(rng); }, distribution_);
    }
};

// #endregion

// #region Vector specialization

template <math::is_vector Vector>
class UniformDistribution<Vector>
{
  public:
    using Component = typename Vector::Component;

    struct HypercubeVolume_
    {
        Vector min{};
        Vector max{};

        template <class Rng>
        Vector
        operator()(Rng &rng) const
        {
            Vector result;
            for (std::size_t i = 0; i < Vector::dimension(); ++i)
            {
                std::uniform_real_distribution<Component> dist(min[i], max[i]);
                result[i] = dist(rng);
            }
            return result;
        }
    };

    struct UnitHypersphereSurface_
    {
        Component radius = Component(1);

        template <class Rng>
        Vector
        operator()(Rng &rng) const
        {
            return internal_random_unit_vector_<Vector>(rng) * radius;
        }
    };

    struct HypersphereSurface_
    {
        Vector    center{};
        Component radius = Component(1);

        template <class Rng>
        Vector
        operator()(Rng &rng) const
        {
            return center + internal_random_unit_vector_<Vector>(rng) * radius;
        }
    };

    struct HypersphereVolume_
    {
        Vector    center{};
        Component radius = Component(1);

        template <class Rng>
        Vector
        operator()(Rng &rng) const
        {
            // N-D volume-uniform: r = radius * u^{1/N}
            std::uniform_real_distribution<Component> u01{ Component(0), Component(1) };
            auto r = radius * std::pow(u01(rng), Component(1) / Component(Vector::dimension()));
            return center + internal_random_unit_vector_<Vector>(rng) * r;
        }
    };

    // N-D cone: random direction within half_angle of the given axis
    struct Cone_
    {
        Vector    direction{};
        Component half_angle{};

        template <class Rng>
        Vector
        operator()(Rng &rng) const
        {
            static_assert(Vector::dimension() >= 2, "Cone requires at least 2D");

            Vector perp = internal_random_perpendicular_unit_vector_<Vector>(direction, rng);

            Component theta = internal_sample_cone_angle_<Vector>(half_angle, rng);

            return direction * std::cos(theta) + perp * std::sin(theta);
        }
    };

    using Variant_ = std::variant<HypercubeVolume_,
                                  UnitHypersphereSurface_,
                                  HypersphereSurface_,
                                  HypersphereVolume_,
                                  Cone_>;

  private:
    Variant_ distribution_ = UnitHypersphereSurface_{};

    // N-D random unit vector via Gaussian method.
    // Gaussian works for any N with O(N) cost and no rejection;
    // rejection sampling on a hypercube becomes exponentially
    // inefficient as N grows.
    template <math::is_vector Vec, class Rng>
    static Vec
    internal_random_unit_vector_(Rng &rng)
    {
        std::normal_distribution<Component> normal;
        Vec                                 v;
        for (auto &c : v)
        {
            c = normal(rng);
        }
        return math::normal(v);
    }

    template <math::is_vector Vec, class Rng>
    static Vec
    internal_random_perpendicular_unit_vector_(const Vec &dir, Rng &rng)
    {
        std::normal_distribution<Component> normal;
        for (;;)
        {
            Vec v;
            for (auto &c : v)
            {
                c = normal(rng);
            }

            Component parallel = math::dot(v, dir);
            for (std::size_t i = 0; i < Vec::dimension(); ++i)
            {
                v[i] -= parallel * dir[i];
            }

            // Near-zero can happen if v was (near-)parallel to dir;
            // loop is simpler than branching on a measure-zero event.
            Component len2{};
            for (const auto &c : v)
            {
                len2 += c * c;
            }
            if (len2 > Component(1.0e-12))
            {
                return v / std::sqrt(len2);
            }
        }
    }

    // Sample theta in [0, half_angle] with PDF ∝ sin^{N-2}(θ).
    // N=2: uniform in angle.  N=3: analytic via CDF inversion.
    // N≥4: rejection sampling against envelope = 1.
    template <math::is_vector Vec, class Rng>
    static Component
    internal_sample_cone_angle_(Component half_angle, Rng &rng)
    {
        constexpr std::size_t                     N = Vec::dimension();
        std::uniform_real_distribution<Component> u01{ Component(0), Component(1) };

        if constexpr (N == 2)
        {
            return u01(rng) * half_angle;
        }
        else if constexpr (N == 3)
        {
            // CDF = (1-cos(θ)) / (1-cos(α))  →  invertible
            return std::acos(Component(1) - u01(rng) * (Component(1) - std::cos(half_angle)));
        }
        else
        {
            constexpr std::size_t exponent = N - 2;
            for (;;)
            {
                Component theta = u01(rng) * half_angle;
                Component p     = std::pow(std::sin(theta), exponent);
                if (u01(rng) <= p)
                {
                    return theta;
                }
            }
        }
    }

  public:
    UniformDistribution() = default;

    template <class Strategy>
    UniformDistribution(Strategy strategy)
        : distribution_(std::move(strategy))
    {
    }

    static UniformDistribution
    hypercube_volume(Vector min, Vector max)
    {
        return HypercubeVolume_{ .min = std::move(min), .max = std::move(max) };
    }

    static UniformDistribution
    unit_hypersphere_surface(Component radius = Component(1))
    {
        return UnitHypersphereSurface_{ .radius = radius };
    }

    static UniformDistribution
    hypersphere_surface(Vector center, Component radius)
    {
        return HypersphereSurface_{ .center = std::move(center), .radius = radius };
    }

    static UniformDistribution
    hypersphere_volume(Vector center, Component radius)
    {
        return HypersphereVolume_{ .center = std::move(center), .radius = radius };
    }

    static UniformDistribution
    cone(Vector direction, Component half_angle)
    {
        return Cone_{ .direction = math::normal(std::move(direction)), .half_angle = half_angle };
    }

    template <class Rng>
    Vector
    operator()(Rng &rng) const
    {
        return std::visit([&](const auto &d) { return d(rng); }, distribution_);
    }
};

// #endregion

} // namespace ome
