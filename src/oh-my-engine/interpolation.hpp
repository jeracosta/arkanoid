#pragma once

#include "oh-my-engine/math/curve.hpp"

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
        return target * curve(progress) + origin * (1.0f - curve(progress));
    }

    void
    flip()
    {
        std::swap(origin, target);
    }
};

} // namespace ome
