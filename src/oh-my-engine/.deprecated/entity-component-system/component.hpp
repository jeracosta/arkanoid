#pragma once

#include <boost/mp11.hpp>

#include "oh-my-engine/entity-component-system/components/index.hpp"

namespace ome::ecs {

// clang-format off
#define X(name) components::name,
#define X_LAST(name) components::name
using Components = boost::mp11::mp_list<
    #include "oh-my-engine/entity-component-system/components/index.def"
>;
#undef X
// clang-format on

} // namespace ome::ecs
