#pragma once

#include <random>

namespace ome::math {

template <typename T>
concept Region = requires(const T &t, const typename T::Vector &p, std::mt19937 &rng) {
    typename T::Vector;
    typename T::ComponentType;
    { T::dimension() } -> std::convertible_to<std::size_t>;
    { t.contains(p) } -> std::same_as<bool>;
    { t.sample_uniform(rng) } -> std::same_as<typename T::Vector>;
};

} // namespace ome::math
