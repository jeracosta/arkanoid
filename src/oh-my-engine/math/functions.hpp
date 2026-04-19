#pragma once

#include <cassert>
#include <cmath>

namespace ome::math {

inline auto
make_lerp(float from, float to)
{
    return [=](float t) { return std::lerp(from, to, t); };
};

inline auto
make_sigmoid(float steepness = 1.0f, float midpoint = 0.0f)
{
    return [=](float t) { return 2.0f / (1.0f + std::exp(-steepness * (t - midpoint))); };
}

} // namespace ome::math
