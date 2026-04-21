#include "component.hpp"

namespace ome::ecs {

inline ComponentTypeIndex
next_type_index_()
{
    static ComponentTypeIndex counter = 0;
    return counter++;
}

template <class Derived>
BaseComponent<Derived>::TypeIndex
type_index()
{
    return next_type_index_();
}

} // namespace ome::ecs
