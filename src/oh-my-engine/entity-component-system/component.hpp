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

// clang-format off
#define X(name) name,
using Components = boost::mp11::mp_list<
    #include "components.def"
>;
#undef X
// clang-format on

static_assert(boost::mp11::mp_all_of<Components, is_component>::value,
              "one or more types are not valid components");

} // namespace ome::ecs
