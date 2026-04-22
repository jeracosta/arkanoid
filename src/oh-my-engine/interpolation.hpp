#pragma once

#include "curve.hpp"

namespace ome {
template <typename T>
struct Interpolation
{
    T     origin;
    T     target;
    Curve curve;

    T
    operator()(float progress) const
    {
        return curve(progress) * target + (1.0f - curve(progress)) * origin;
    }

    void
    flip()
    {
        std::swap(origin, target);
    }
};

} // namespace ome
