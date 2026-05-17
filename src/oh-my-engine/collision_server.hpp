#pragma once

#include <vector>

namespace ome {

class HitboxNode;

class CollisionServer
{
  public:
    void
    register_hitbox(HitboxNode &node);

    void
    unregister_hitbox(HitboxNode &node);

    void
    process_collisions();

  private:
    std::vector<HitboxNode *> nodes_;
};

} // namespace ome
