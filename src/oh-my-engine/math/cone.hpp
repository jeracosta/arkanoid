#pragma once

#include <optional>

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

template <std::size_t Dimension, typename Component = float>
class Cone
{
  public:
    using Vector        = Vector<Dimension, Component>;
    using ComponentType = Component;

    static constexpr std::size_t
    dimension()
    {
        return Dimension;
    }

    Cone() = default;

    Cone(Vector                   apex,
         Vector                   direction,
         Component                half_angle,
         std::optional<Component> height = std::nullopt)
        : apex_(std::move(apex)),
          direction_(math::normalized(std::move(direction))),
          half_angle_(half_angle),
          height_(height)
    {
    }

    const Vector &
    apex() const
    {
        return apex_;
    }

    const Vector &
    direction() const
    {
        return direction_;
    }

    Component
    half_angle() const
    {
        return half_angle_;
    }

    const std::optional<Component> &
    height() const
    {
        return height_;
    }

    void
    displace(const Vector &delta)
    {
        apex_ += delta;
    }

    bool
    contains(const Vector &point) const
    {
        auto relative = point - apex_;

        if (!height_)
        {
            if (norm(relative) == 0)
            {
                return true;
            }

            return dot(normalized(relative), direction_) >= std::cos(half_angle_);
        }

        auto t          = dot(relative, direction_);
        auto orthogonal = relative - direction_ * t;
        auto h          = *height_;

        return t >= 0 && t <= h && math::norm(orthogonal) <= t * std::tan(half_angle_);
    }

    // Generates a random point uniformly distributed within the cone volume.
    template <class Rng>
    Vector
    sample_uniform(Rng &rng) const
    {
        auto uniform = [&rng]
        {
            static auto distribution = std::uniform_real_distribution<Component>{ 0, 1 };
            return distribution(rng);
        };

        Vector orthogonal = orthonormalize(random_unit_vector<Vector>(rng), direction_);

        if (!height_)
        {
            auto theta = sample_angle_(half_angle_, rng);
            return apex_ + direction_ * std::cos(theta) + orthogonal * std::sin(theta);
        }

        // Inverse CDF sampling for uniform distribution in a finite cone.
        // Height from apex: PDF ~ t^(D-1), CDF = (t/h)^D → t = h * u^(1/D)
        auto height_coord = *height_ * std::pow(uniform(), 1.0 / Dimension);

        // Disk radius at this height grows linearly from apex
        auto radius_upper_bound = height_coord * std::tan(half_angle_);

        // Uniform disk sampling at this height: PDF ~ r, CDF = (r/r_max)^2 → r = r_max * sqrt(v)
        auto radius_coord = radius_upper_bound * std::sqrt(uniform());

        return apex_ + direction_ * height_coord + orthogonal * radius_coord;
    }

  private:
    Vector                   apex_{};
    Vector                   direction_{};
    Component                half_angle_{};
    std::optional<Component> height_;

    template <class Rng>
    Component
    sample_angle_(Component half_angle, Rng &rng) const
    {
        auto uniform_dist = std::uniform_real_distribution<Component>{};
        auto uniform      = uniform_dist(rng);

        if constexpr (Dimension == 2)
        {
            return uniform * half_angle;
        }
        else if constexpr (Dimension == 3)
        {
            return std::acos(1 - uniform * (1 - std::cos(half_angle)));
        }
        else
        {
            static_assert(!"Unsupported dimension");
        }
    }
};

} // namespace ome::math
