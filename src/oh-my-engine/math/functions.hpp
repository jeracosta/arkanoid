#pragma once

#include <cassert>
#include <cmath>

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

inline auto
make_lerp(float from, float to)
{
    return [=](float t) { return std::lerp(from, to, t); };
};

template <is_vector Vector>
inline auto
make_lerp(const Vector &from, const Vector &to)
{
    return transform(make_lerp, from, to);
}

inline auto
make_sigmoid(float steepness = 1.0f, float midpoint = 0.0f)
{
    return [=](float t) { return 2.0f / (1.0f + std::exp(-steepness * (t - midpoint))); };
}

} // namespace ome::math
