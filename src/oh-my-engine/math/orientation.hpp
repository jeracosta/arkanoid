#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Orientation
{
  private:
    glm::quat quat_{ 1, 0, 0, 0 }; // identity

  public:
    Orientation() = default;

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

    void
    steer_yaw(float delta)
    {
        glm::quat q = glm::angleAxis(delta, glm::vec3(0, 1, 0));

        quat_ = glm::normalize(q * quat_);
    }

    void
    steer_pitch(float delta)
    {
        glm::vec3 right = quat_ * glm::vec3(1, 0, 0);

        glm::quat q = glm::angleAxis(delta, right);

        quat_ = glm::normalize(q * quat_);
    }

    void
    steer_roll(float delta)
    {
        glm::vec3 forward = quat_ * glm::vec3(0, 0, 1);

        glm::quat q = glm::angleAxis(delta, forward);

        quat_ = glm::normalize(q * quat_);
    }

#define DEFINE_DIR_GETTER(name)                                                                    \
    Vec3f name() const                                                                             \
    {                                                                                              \
        return Vec3f(quat_ * glm::vec3(directions::name));                                         \
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
};

} // namespace ome
