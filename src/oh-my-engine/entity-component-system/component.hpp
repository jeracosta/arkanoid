#pragma once

#include <boost/mp11.hpp>
#include <concepts>
#include <cstddef>

namespace ome::ecs {

using ComponentTypeIndex = std::size_t;

template <class Derived>
class BaseComponent
{
    static_assert(std::derived_from<Derived, BaseComponent<Derived>>);

  public:
    static ComponentTypeIndex
    type_index();
};

template <class T>
concept IsComponent = std::derived_from<T, BaseComponent<T>>;

using Components = boost::mp11::mp_list<>;

} // namespace ome::ecs
