#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Orientation
{
  private:
    glm::quat quat_;

  public:
    explicit constexpr Orientation(const glm::quat &quat)
        : quat_(quat)
    {
    }

    static consteval Orientation
    identity()
    {
        return Orientation(glm::quat{ 1, 0, 0, 0 });
    }

    constexpr Orientation()
        : Orientation(identity())
    {
    }

    operator glm::quat() const
    {
        return quat_;
    }

    glm::quat
    quat() const
    {
        return quat_;
    }

    glm::mat4
    matrix() const
    {
        return glm::mat4_cast(quat_);
    }

#define DEFINE_STEER(name, x, y, z)                                                                \
    Orientation &steer_##name(float delta)                                                         \
    {                                                                                              \
        glm::quat q = glm::angleAxis(delta, glm::vec3(x, y, z));                                   \
        quat_       = glm::normalize(q * quat_);                                                   \
        return *this;                                                                              \
    }
    DEFINE_STEER(pitch, 1, 0, 0)
    DEFINE_STEER(yaw, 0, 1, 0)
    DEFINE_STEER(roll, 0, 0, 1)
#undef DEFINE_STEER

#define DEFINE_DIR_GETTER(name)                                                                    \
    Vec3f name() const                                                                             \
    {                                                                                              \
        return Vec3f(quat_ * glm::vec3(ome::name));                                                \
    }
    DEFINE_DIR_GETTER(forward)
    DEFINE_DIR_GETTER(backward)
    DEFINE_DIR_GETTER(up)
    DEFINE_DIR_GETTER(down)
    DEFINE_DIR_GETTER(left)
    DEFINE_DIR_GETTER(right)
#undef DEFINE_DIR_GETTER

    float
    yaw() const
    {
        return glm::eulerAngles(quat_).y;
    }

    float
    pitch() const
    {
        return glm::eulerAngles(quat_).x;
    }

    float
    roll() const
    {
        return glm::eulerAngles(quat_).z;
    }

    Orientation &
    rotate(float angle, const Vec3f &axis)
    {
        glm::quat q = glm::angleAxis(angle, glm::vec3(axis));
        quat_       = glm::normalize(q * quat_);
        return *this;
    }

    Orientation
    operator*(float s) const
    {
        return Orientation(quat_ * s);
    }

    Orientation
    operator+(const Orientation &other) const
    {
        return Orientation(quat_ + other.quat_);
    }

    inline Orientation
    operator*(const Orientation &other) const
    {
        return Orientation(glm::normalize(quat_ * other.quat_));
    }
};

inline Orientation
inverse_of(const Orientation &orientation)
{
    return Orientation(glm::inverse(orientation.quat()));
}

inline Vec3f
operator*(const Orientation &orientation, const Vec3f &vector)
{
    return orientation.quat() * glm::vec3(vector);
}

} // namespace ome
