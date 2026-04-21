#pragma once

#include <boost/mp11.hpp>
#include <concepts>
#include <cstddef>

namespace ome::ecs {

using ComponentTypeIndex = std::size_t;

template <class Derived>
class BaseComponent
{
};

template <class T>
concept IsComponent = std::derived_from<T, BaseComponent<T>> && requires {
    typename T::Dependencies;
} && boost::mp11::mp_is_list<typename T::Dependencies>::value;

template <class T>
struct is_component : std::bool_constant<IsComponent<T>>
{
};
} // namespace ome::ecs
