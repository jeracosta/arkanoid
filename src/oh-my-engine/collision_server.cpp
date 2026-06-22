#include "oh-my-engine/collision_server.hpp"

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "arkanoid/nodes/projectile.hpp"

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

    // HACK: temporal ad-hoc optimization due to lack of time.
    // Only checks collisions involving projectile hitboxes.
    // Remove this block and restore generic pair processing below.
    {
        for (auto *node : nodes_)
        {
            auto *projectile = dynamic_cast<arkanoid::ProjectileNode::HitboxNode *>(node);
            if (!projectile)
            {
                continue;
            }

            for (auto *other : nodes_)
            {
                if (other == node)
                {
                    continue;
                }

                if (overlaps(node->hitbox<Space::World>(), other->hitbox<Space::World>()))
                {
                    node->on_collision_(*other);
                    other->on_collision_(*node);
                }
            }
        }
        return;
    }
    // end HACK

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
