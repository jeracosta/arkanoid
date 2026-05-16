#pragma once

#include "oh-my-engine/nodes/mixins/despawning.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

inline constexpr auto exceeds_despawn_distance = [](const ome::TransformNode &node)
{ return norm(node.transform<ome::Space::World>().position) > despawn_distance; };

template <std::derived_from<ome::Node> TBase>
using DistanceCulled = ome::Despawning<TBase, decltype(exceeds_despawn_distance)>;

} // namespace soccernoid
