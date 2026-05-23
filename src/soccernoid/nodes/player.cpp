#include "soccernoid/nodes/player.hpp"

namespace soccernoid {

void
PlayerNode::clamp_to_bounds_()
{
    if (!map_node_)
    {
        return;
    }
    auto area = map_node_->area();

    update_transform<ome::Space::Local>([&](auto &t)
    {
        t.position[0] = std::clamp(t.position[0], -area[0] + player_radius_, area[0] - player_radius_);
        t.position[2] = std::clamp(t.position[2], -area[1] + player_radius_, area[1] - player_radius_);
    });
}

} // namespace soccernoid
