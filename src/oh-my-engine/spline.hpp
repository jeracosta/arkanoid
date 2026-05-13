#pragma once

#include <algorithm>
#include <vector>

#include "oh-my-engine/curve.hpp"

namespace ome {

template <typename T>
struct Spline
{
    struct ControlPoint
    {
        float position;
        T     value;
    };

    struct Knot
    {
        float position;
        T     value;
        T     tangent;
    };
};

template <typename T>
class SplineCurve : public Curve<T>
{
    std::vector<typename Spline<T>::Knot> knots_;

    static T
    segment_(const typename Spline<T>::Knot &a, const typename Spline<T>::Knot &b, float t)
    {
        float dt = b.position - a.position;
        float s  = (t - a.position) / dt;
        float s2 = s * s;
        float s3 = s2 * s;

        float h00 = 2.0f * s3 - 3.0f * s2 + 1.0f;
        float h10 = s3 - 2.0f * s2 + s;
        float h01 = -2.0f * s3 + 3.0f * s2;
        float h11 = s3 - s2;

        return a.value * h00 + a.tangent * (dt * h10) + b.value * h01 + b.tangent * (dt * h11);
    }

  public:
    SplineCurve() = default;

    explicit SplineCurve(std::vector<typename Spline<T>::Knot> knots)
        : knots_(std::move(knots))
    {
    }

    T
    operator()(float t) const override
    {
        if (knots_.empty())
            return T{};

        if (t <= knots_.front().position)
            return knots_.front().value;

        if (t >= knots_.back().position)
            return knots_.back().value;

        auto it = std::ranges::upper_bound(knots_, t, {}, &Spline<T>::Knot::position);

        return segment_(*(it - 1), *it, t);
    }

    static std::vector<typename Spline<T>::Knot>
    catmull_rom(std::initializer_list<typename Spline<T>::ControlPoint> control_points)
    {
        auto n      = control_points.size();
        auto *begin = control_points.begin();
        auto knots  = std::vector<typename Spline<T>::Knot>();
        knots.reserve(n);

        auto tangent = [&](std::size_t i) -> T
        {
            if (n == 1)
                return T{};

            if (i == 0)
                return (begin[1].value - begin[0].value)
                       / (begin[1].position - begin[0].position);

            if (i == n - 1)
                return (begin[i].value - begin[i - 1].value)
                       / (begin[i].position - begin[i - 1].position);

            return (begin[i + 1].value - begin[i - 1].value)
                   / (begin[i + 1].position - begin[i - 1].position);
        };

        for (std::size_t i = 0; i < n; ++i)
        {
            knots.push_back({ .position = begin[i].position,
                              .value    = begin[i].value,
                              .tangent  = tangent(i) });
        }
        return knots;
    }
};

} // namespace ome
