#include "oh-my-engine/collision_server.hpp"

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"

namespace ome {

void
CollisionServer::register_hitbox(HitboxNode &node)
{
    nodes_.push_back(&node);
}

void
CollisionServer::unregister_hitbox(HitboxNode &node)
{
    std::erase(nodes_, &node);
}

void
CollisionServer::process_collisions()
{
    using namespace std::views;

    auto indices = iota(std::size_t{ 0 }, nodes_.size());

    auto index_pairs = cartesian_product(indices, indices)
                       | filter([](const auto &t) { return std::get<0>(t) < std::get<1>(t); });

    auto node_pairs = index_pairs
                      | std::views::transform([this](const auto &t)
    {
        auto [i, j] = t;
        return std::make_pair(std::ref(nodes_[i]), std::ref(nodes_[j]));
    });

    for (auto [a, b] : node_pairs)
    {
        if (overlaps(a->hitbox<Space::World>(), b->hitbox<Space::World>()))
        {
            a->on_collision_(*b);
            b->on_collision_(*a);
        }
    }
}

} // namespace ome
