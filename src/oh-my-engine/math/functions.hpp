#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>

#include "oh-my-engine/math/vector.hpp"

namespace ome::math {

inline auto
make_lerp(float from, float to)
{
    return [=](auto t) { return std::lerp(from, to, t); };
};

inline auto
make_sigmoid(float steepness = 1.0f, float midpoint = 0.0f)
{
    return [=](auto t) { return 2.0f / (1.0f + std::exp(-steepness * (t - midpoint))); };
}

inline auto
make_smoothstep(float from, float to, float steepness = 1.0f)
{
    return [=](float t)
    {
        float x = std::clamp(t, 0.0f, 1.0f);

        float a = std::pow(x, steepness);
        float b = std::pow(1.0f - x, steepness);

        float s = a / (a + b);

        s = s * s * (3.0f - 2.0f * s);

        return from + (to - from) * s;
    };
}

#define DEFINE_VECTOR_OVERLOAD(FUNC)                                                               \
    template <is_vector Vector, typename... Args>                                                  \
    inline auto FUNC(const Vector &from, const Vector &to, Args &&...args)                         \
    {                                                                                              \
        return [=, args = std::tuple<Args...>(std::forward<Args>(args)...)](float t)               \
        {                                                                                          \
            return transform(                                                                      \
                [&, t](auto a, auto b)                                                             \
            {                                                                                      \
                return std::apply([&](auto &&...unpacked) { return FUNC(a, b, unpacked...)(t); },  \
                                  args);                                                           \
            },                                                                                     \
                from,                                                                              \
                to);                                                                               \
        };                                                                                         \
    }
DEFINE_VECTOR_OVERLOAD(make_lerp)
DEFINE_VECTOR_OVERLOAD(make_sigmoid)
DEFINE_VECTOR_OVERLOAD(make_smoothstep)
#undef DEFINE_VECTOR_OVERLOAD

} // namespace ome::math
