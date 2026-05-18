#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct Transform
{
    Vec3f       position    = { 0.0f };
    Orientation orientation = Orientation::identity();
    Vec3f       scale       = { 1.0f };
};

} // namespace ome
