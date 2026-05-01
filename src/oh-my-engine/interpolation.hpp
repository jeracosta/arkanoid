#pragma once

#include "oh-my-engine/math/curve.hpp"

namespace ome {
template <typename T>
struct Interpolation
{
    T     from;
    T     to;
    Curve curve;

    T
    operator()(float progress) const
    {
        return to * curve(progress) + from * (1.0f - curve(progress));
    }

    void
    flip()
    {
        std::swap(from, to);
    }
};

} // namespace ome
