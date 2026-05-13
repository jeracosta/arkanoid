#pragma once

#include <random>

#include "oh-my-engine/math/ball.hpp"
#include "oh-my-engine/math/box.hpp"
#include "oh-my-engine/math/cone.hpp"
#include "oh-my-engine/math/sphere.hpp"

namespace ome::math {

template <typename T>
concept Region = requires(const T &t, const typename T::Vector &p, std::mt19937 &rng) {
    typename T::Vector;
    typename T::ComponentType;
    { T::dimension() } -> std::convertible_to<std::size_t>;
    { t.contains(p) } -> std::same_as<bool>;
    { t.sample_uniform(rng) } -> std::same_as<typename T::Vector>;
};

// WARN: Needs manual update when new region types are added.
// TODO: Improve maintainability.
// NOTE: std::variant allows us to avoid dynamic memory allocation and virtual function calls.
template <std::size_t N, typename C = float>
using AnyRegion = std::variant<Sphere<N, C>, Ball<N, C>, Box<N, C>, Cone<N, C>>;

template <std::size_t N, typename C, class Rng>
Vector<N, C>
sample_uniform(const AnyRegion<N, C> &region, Rng &rng)
{
    return std::visit([&](const auto &r) { return r.sample_uniform(rng); }, region);
}

template <std::size_t N, typename C>
bool
contains(const AnyRegion<N, C> &region, const Vector<N, C> &point)
{
    return std::visit([&](const auto &r) { return r.contains(point); }, region);
}

} // namespace ome::math
