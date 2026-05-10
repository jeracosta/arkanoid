#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "soccernoid/nodes/mixins/distance_culled.hpp"
#include "soccernoid/nodes/mixins/falling.hpp"

namespace soccernoid {

class BallNode : public DistanceCulled<Falling<ome::KinematicNode>>
{
  private:
    using Base_ = DistanceCulled<Falling<ome::KinematicNode>>;

    ome::Color color_           = ome::Color::rgb(255, 0, 0);
    float      radius_          = 0.10f; // Futsal size 4 ball: circumference 62-64cm, radius ~0.10m
    float      elasticity_      = 0.8f;
    float      speed_threshold_ = 0.1f;

    ome::HitboxNode *terrain_;

    // spawn at the futsal penalty point, ball clearing the ground
    static constexpr ome::Vec3f spawn_position = { 0.0f, 0.5f, 0.0f };

    ome::HitboxComponent hitbox_{
        .min = { -0.10f, -0.10f, -0.10f },
        .max = { 0.10f, 0.10f, 0.10f },
    };

    ome::HitboxComponent
    hitbox_world_() const
    {
        auto transform = world_transform();
        return { .min = transform.to_world(hitbox_.min), .max = transform.to_world(hitbox_.max) };
    }

  public:
    explicit BallNode(ome::HitboxNode &terrain)
        : terrain_(&terrain)
    {
        auto transform     = local_transform();
        transform.position = spawn_position;
        set_local_transform(transform);
    }

    void
    on_tick_() override
    {
        Base_::on_tick_();

        bounce_();
        render_();
    }

  private:
    void
    bounce_()
    {
        if (!ome::are_colliding(hitbox_world_(), terrain_->hitbox_world()))
        {
            return;
        }

        auto transform        = local_transform();
        transform.position[1] = radius_;
        set_local_transform(transform);

        float fall_speed = dot(velocity(), ome::up);
        if (fall_speed >= 0.0f)
        {
            return;
        }

        if (std::abs(fall_speed) < speed_threshold_)
        {
            set_velocity({ velocity()[0], 0.0f, velocity()[2] });
            return;
        }

        set_velocity({ velocity()[0], -fall_speed * elasticity_, velocity()[2] });
    }

    void
    render_()
    {
        auto position = world_transform().position;

        glColor(color_);
        glPushMatrix();
        {
            GLUquadric *q = gluNewQuadric();
            glTranslatef(position[0], position[1], position[2]);
            gluSphere(q, radius_, 32, 32);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }
};

} // namespace soccernoid
