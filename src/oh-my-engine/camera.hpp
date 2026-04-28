#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/ext/vector_float3.hpp>

#include "oh-my-engine/events.hpp"
#include "oh-my-engine/math/orientation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/member_forwarding_macro.hpp"

namespace ome {

struct ProjectionUpdatedEvent;

struct Camera : private EventDispatcher<ProjectionUpdatedEvent>
{
  public:
    struct Settings
    {
        struct Projection
        {
            float fov_degrees  = 45.0f;
            float aspect_ratio = 4.0f / 3.0f;
            float near         = 0.1f;
            float far          = 100.0f;
        };

        Vec3f       target      = { 0, 0, 0 };
        float       distance    = { 1.0f };
        Orientation orientation = {};
        Projection  projection  = {};
    };

    // "Settings" implies internal data mutable ("directly"; i.e. some kind of setter) by the user.
    // "Configuration" implies data that is received at construction to initialice internal state.
    // In this case, those are the same: the configuration is the initial settings.
    using Configuration = Settings;

  private:
    Settings settings_;

    Vec3f                &target_      = settings_.target;
    float                &distance_    = settings_.distance;
    Settings::Projection &projection_  = settings_.projection;
    Orientation          &orientation_ = settings_.orientation;

  public:
    Camera(Settings initial_settings)
        : settings_(std::move(initial_settings))
    {
    }

    const Orientation &
    orientation() const
    {
        return orientation_;
    }

    void
    orientate(const Orientation &new_orientation)
    {
        orientation_ = new_orientation;
    }

    FORWARD_TO_MEMBER(steer_pitch, orientation_)
    FORWARD_TO_MEMBER(steer_roll, orientation_)
    FORWARD_TO_MEMBER(steer_yaw, orientation_)
    FORWARD_TO_MEMBER(backward, orientation_)
    FORWARD_TO_MEMBER(down, orientation_)
    FORWARD_TO_MEMBER(forward, orientation_)
    FORWARD_TO_MEMBER(left, orientation_)
    FORWARD_TO_MEMBER(right, orientation_)
    FORWARD_TO_MEMBER(up, orientation_)
    FORWARD_TO_MEMBER(pitch, orientation_)
    FORWARD_TO_MEMBER(roll, orientation_)
    FORWARD_TO_MEMBER(rotate, orientation_)

    Vec3f
    target() const
    {
        return target_;
    }

    void
    target(const Vec3f &new_value)
    {
        target_ = new_value;
    }

    void
    move_target(const Vec3f &delta)
    {
        target_ += delta;
    }

    float
    distance() const
    {
        return distance_;
    }

    void
    distance(float new_value)
    {
        distance_ = new_value;
    }

    Vec3f
    position() const
    {
        return target_ - forward() * distance_;
    }

    friend Vec3f
    to_camera_space(const Vec3f &world_vector, const Camera &camera)
    {
        glm::vec3 v            = glm::vec3(world_vector - camera.position());
        glm::vec3 camera_space = glm::conjugate(camera.orientation().quat()) * v;
        return Vec3f(camera_space);
    }

    friend Vec3f
    from_camera_space(const Vec3f &camera_space_vector, const Camera &camera)
    {
        glm::vec3 v     = glm::vec3(camera_space_vector);
        glm::vec3 world = camera.orientation().quat() * v + glm::vec3(camera.position());
        return Vec3f(world);
    }

    using EventDispatcher<ProjectionUpdatedEvent>::bind;

    float
    fov_degrees() const
    {
        return projection_.fov_degrees;
    }

    void
    fov_degrees(float new_value);

    float
    aspect_ratio() const
    {
        return projection_.aspect_ratio;
    }

    void
    aspect_ratio(float new_value);

    float
    near() const
    {
        return projection_.near;
    }

    void
    near(float new_value);

    float
    far() const
    {
        return projection_.far;
    }

    void
    far(float new_value);
};

struct ProjectionUpdatedEvent
{
    Camera::Settings::Projection new_projection;
};

inline void
Camera::fov_degrees(float new_value)
{
    projection_.fov_degrees = new_value;
    emit(ProjectionUpdatedEvent{ projection_ });
}

inline void
Camera::aspect_ratio(float new_value)
{
    projection_.aspect_ratio = new_value;
    emit(ProjectionUpdatedEvent{ projection_ });
}

inline void
Camera::near(float new_value)
{
    projection_.near = new_value;
    emit(ProjectionUpdatedEvent{ projection_ });
}

inline void
Camera::far(float new_value)
{
    projection_.far = new_value;
    emit(ProjectionUpdatedEvent{ projection_ });
}

namespace open_gl {

inline void
look_at(const ome::Camera &camera)
{
    auto position = camera.position();
    auto target   = camera.target();
    auto up       = camera.up();

    gluLookAt(position[0],
              position[1],
              position[2],
              target[0],
              target[1],
              target[2],
              up[0],
              up[1],
              up[2]);
}

} // namespace open_gl

} // namespace ome
