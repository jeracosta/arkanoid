#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/ext/vector_float3.hpp>

#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct Camera
{
    Vec3f       target      = {}; // point you look at
    Orientation orientation = {}; // rotation around target
    float       distance;         // zoom

    Vec3f
    position() const
    {
        return target - orientation.forward() * distance;
    }
};

} // namespace ome

inline void
gluLookAt(const ome::Camera &camera)
{
    auto position = camera.position();
    auto up       = camera.orientation.up();

    gluLookAt(position[0],
              position[1],
              position[2],
              camera.target[0],
              camera.target[1],
              camera.target[2],
              up[0],
              up[1],
              up[2]);
}
