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

static_assert(boost::mp11::mp_all_of<Components, is_component>::value,
              "one or more types are not valid components");
} // namespace ome::ecs
