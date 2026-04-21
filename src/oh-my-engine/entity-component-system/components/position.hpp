#pragma once

#include "oh-my-engine/entity-component-system/component.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome::ecs::components {

struct Position : public BaseComponent<Position>
{
    Vec3f vector;

    using Dependencies = boost::mp11::mp_list<>;
};

} // namespace ome::ecs::components
