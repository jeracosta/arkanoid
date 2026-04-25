#pragma once

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {
class GravityNode : public Node
{
  private:
    TransformNode *target_;

  public:
    Vec3f force = down * 9.81f;

    void
    on_mount_() override
    {
        target_ = find_ancestor<TransformNode>(this);

        if (!target_)
        {
            throw std::runtime_error("GravityNode failed to find a TransformNode ancestor.");
        }
    }

    void
    tick_() override
    {
        target_->position += force * game()->time.delta();
    }
};
} // namespace ome
