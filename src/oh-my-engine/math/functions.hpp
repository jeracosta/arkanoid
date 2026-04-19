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

inline auto
make_sigmoid(float steepness = 1.0f, float midpoint = 0.0f)
{
    return [=](float t) { return 2.0f / (1.0f + std::exp(-steepness * (t - midpoint))); };
}

#define DEFINE_VECTOR_OVERLOAD(FUNC)                                                               \
    template <is_vector Vector>                                                                    \
    inline auto FUNC(const Vector &from, const Vector &to)                                         \
    {                                                                                              \
        return [=](float t)                                                                        \
        { return transform([t](auto a, auto b) { return FUNC(a, b)(t); }, from, to); };            \
    }
DEFINE_VECTOR_OVERLOAD(make_lerp)
DEFINE_VECTOR_OVERLOAD(make_sigmoid)
#undef DEFINE_VECTOR_OVERLOAD

} // namespace ome::math
