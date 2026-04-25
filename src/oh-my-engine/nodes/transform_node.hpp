#pragma once

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"

namespace ome {
class TransformNode : public Node
{
  public:
    Vec3f       position;
    Orientation orientation;
    Vec3f       scale;
};
} // namespace ome
