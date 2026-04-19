#pragma once

#include <cassert>
#include <cmath>

namespace ome::math {

inline auto
make_lerp(float from, float to)
{
    return [=](float t) { return std::lerp(from, to, t); };
};

} // namespace ome::math
