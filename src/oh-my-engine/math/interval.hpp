#pragma once

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

namespace math {

template <std::size_t Dimension, typename Component = float>
class Interval
{
  public:
    using Vector        = Vector<Dimension, Component>;
    using ComponentType = Component;

    struct Bounds
    {
        Vector min;
        Vector max;
    };

    static constexpr std::size_t
    dimension()
    {
        return Dimension;
    }

    Interval() = default;

    constexpr Interval(Bounds bounds)
        : min_(std::move(bounds.min)),
          max_(std::move(bounds.max))
    {
    }

    // #region Factory methods

    static constexpr Interval
    from_bounds(Bounds bounds)
    {
        return { bounds };
    }

    static constexpr Interval
    from_bounds(const Vector &min, const Vector &max)
    {
        return from_bounds(Bounds{ min, max });
    }

    static constexpr Interval
    from_size_location(const Vector &side_length, const Vector &location)
    {
        Vector half_size = side_length / Component(2);
        return from_bounds(location - half_size, location + half_size);
    }

    static constexpr Interval
    from_size(const Vector &side_length)
    {
        return from_size_location(side_length, Vector{ 0 });
    }

    static constexpr Interval
    degenerate(const Vector &single_element)
    {
        return from_bounds(single_element, single_element);
    }

    // #endregion

    // #region Accessors

    Bounds
    bounds() const
    {
        return { min_, max_ };
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

    Vec3f
    size() const
    {
        return max_ - min_;
    }

    inline float
    width() const
    {
        return dot(size(), ome::right);
    }

    inline float
    height() const
    {
        return dot(size(), ome::up);
    }

    inline float
    length() const
    {
        return dot(size(), ome::forward);
    }

    Vector
    center() const
    {
        return (min_ + max_) / Component(2);
    }

    // Lazy range over the corners of the box.
    // Order: (min, min, ...), (max, min, ...), (min, max, ...), (max, max, ...), ...
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

    // #endregion

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
    overlaps(const Interval &a, const Interval &b)
    {
        constexpr auto compare = [](auto &a, auto &b)
        {
            return component_wise(std::less{}, a.min(), b.max());
        };

        return compare(a, b) && compare(b, a);
    }

    // clang-format on

    friend Vector
    projection(const Vector &point, const Interval &interval)
    {
        auto [min, max] = interval.bounds();

        auto clamp = [](auto x, auto min, auto max) { return std::clamp(x, min, max); };

        return zip_transform(clamp, point, min, max);
    }

    friend Vector
    overlap_depth(const Interval &a, const Interval &b)
    {
        auto overlap_min = zip_transform(std::ranges::max, a.min(), b.min());
        auto overlap_max = zip_transform(std::ranges::min, a.max(), b.max());

        return overlap_max - overlap_min;
    }

  private:
    // Constructor is private to ensure clearer factory methods are used instead.
    constexpr Interval(Vector min, Vector max)
        : min_(std::move(min)),
          max_(std::move(max))
    {
    }

    Vector min_{};
    Vector max_{};
};

} // namespace math

using Box  = math::Interval<3>;
using Rect = math::Interval<2>;

struct BoxFace
{
    std::array<Vec3f, 4> corners;
    Vec3f                normal;
};

template <typename T>
struct BoxFaces
{
    T front;
    T back;
    T left;
    T right;
    T top;
    T bottom;
};

inline BoxFaces<BoxFace>
faces_of(const Box &box)
{
    auto [min, max] = box.bounds();

    return BoxFaces<BoxFace>{
        .front = BoxFace{
            .corners = {
                Vec3f{ min[0], min[1], max[2] },
                Vec3f{ max[0], min[1], max[2] },
                Vec3f{ max[0], max[1], max[2] },
                Vec3f{ min[0], max[1], max[2] },
            },
            .normal = { 0.0f, 0.0f, 1.0f }
        },

        .back = BoxFace{
            .corners = {
                Vec3f{ max[0], min[1], min[2] },
                Vec3f{ min[0], min[1], min[2] },
                Vec3f{ min[0], max[1], min[2] },
                Vec3f{ max[0], max[1], min[2] },
            },
            .normal = { 0.0f, 0.0f, -1.0f }
        },

        .left = BoxFace{
            .corners = {
                Vec3f{ min[0], min[1], min[2] },
                Vec3f{ min[0], min[1], max[2] },
                Vec3f{ min[0], max[1], max[2] },
                Vec3f{ min[0], max[1], min[2] },
            },
            .normal = { -1.0f, 0.0f, 0.0f }
        },

        .right = BoxFace{
            .corners = {
                Vec3f{ max[0], min[1], max[2] },
                Vec3f{ max[0], min[1], min[2] },
                Vec3f{ max[0], max[1], min[2] },
                Vec3f{ max[0], max[1], max[2] },
            },
            .normal = { 1.0f, 0.0f, 0.0f }
        },

        .top = BoxFace{
            .corners = {
                Vec3f{ min[0], max[1], max[2] },
                Vec3f{ max[0], max[1], max[2] },
                Vec3f{ max[0], max[1], min[2] },
                Vec3f{ min[0], max[1], min[2] },
            },
            .normal = { 0.0f, 1.0f, 0.0f }
        },

        .bottom = BoxFace{
            .corners = {
                Vec3f{ min[0], min[1], min[2] },
                Vec3f{ max[0], min[1], min[2] },
                Vec3f{ max[0], min[1], max[2] },
                Vec3f{ min[0], min[1], max[2] },
            },
            .normal = { 0.0f, -1.0f, 0.0f }
        }
    };
}

} // namespace ome
