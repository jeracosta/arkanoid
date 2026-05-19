#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

// TODO: Turn struct into a class with glm::mat4 storage?

struct Transform
{
    Vec3f       position    = { 0.0f };
    Orientation orientation = Orientation::identity();
    Vec3f       scale       = { 1.0f };

    operator glm::mat4() const
    {
        return glm::translate(glm::mat4(1.0f), glm::vec3(position)) * orientation.matrix()
               * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    }
};

inline Vec3f
operator*(const Transform &transform, const Vec3f &vector)
{
    return transform.position + transform.orientation * (transform.scale * vector);
}

inline Transform
operator*(const Transform &lhs, const Transform &rhs)
{
    return {
        .position    = lhs * rhs.position,
        .orientation = lhs.orientation * rhs.orientation,
        .scale       = lhs.scale * rhs.scale,
    };
}

inline Transform
inverse_of(const Transform &transform)
{
    auto inverse_orientation = inverse_of(transform.orientation);
    auto inverse_scale       = Vec3f{ 1 } / transform.scale;
    auto inverse_position    = inverse_orientation * (inverse_scale * -transform.position);

    return {
        .position    = inverse_position,
        .orientation = inverse_orientation,
        .scale       = inverse_scale,
    };
}

} // namespace ome
