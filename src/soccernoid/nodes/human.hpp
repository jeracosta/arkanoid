#pragma once

#include <GL/gl.h>
#include <GL/glu.h>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

// A simplified human figure: ellipsoid body + sphere head, with a proper hitbox.
// The body ellipsoid is centered at ground level so its bottom half extends
// below y=0 (underground) and is hidden by the terrain.
class HumanNode : public ome::HitboxNode
{
  public:
    struct Configuration
    {
        ome::Color jersey_color = colors.red_kit;
        ome::Color head_color   = colors.skin;
        ome::Vec3f position     = { 0.0f, 0.0f, 0.0f };
    };

  private:
    static constexpr float body_radius_ = 0.50f;
    static constexpr float body_height_ = 2.92f; // full ellipsoid height
    static constexpr float head_radius_ = 0.24f;
    // Above-ground portion
    static constexpr float visible_body_top_ = body_height_ * 0.5f;
    static constexpr float total_height_     = visible_body_top_ + 2.0f * head_radius_;

    ome::Color jersey_color_;
    ome::Color head_color_;

  public:
    HumanNode()
        : HitboxNode({
              { -body_radius_, 0.0f, -body_radius_ },
              { body_radius_, total_height_, body_radius_ },
          }),
          jersey_color_(colors.red_kit),
          head_color_(colors.skin)
    {
    }

    explicit HumanNode(const Configuration &config)
        : HitboxNode({
              { -body_radius_, 0.0f, -body_radius_ },
              { body_radius_, total_height_, body_radius_ },
          }),
          jersey_color_(config.jersey_color),
          head_color_(config.head_color)
    {
        auto transform     = local_transform();
        transform.position = config.position;
        set_local_transform(transform);
    }

    void
    on_tick_() override
    {
        render_();
    }

  private:
    void
    render_()
    {
        auto pos = world_transform().position;

        // Body — ellipsoid centered at ground level (bottom half underground)
        glColor(jersey_color_);
        glPushMatrix();
        {
            glTranslatef(pos[0], pos[1], pos[2]);
            glScalef(1.0f, body_height_ / (2.0f * body_radius_), 1.0f);
            GLUquadric *q = gluNewQuadric();
            gluQuadricNormals(q, GLU_SMOOTH);
            gluSphere(q, body_radius_, 16, 16);
            gluDeleteQuadric(q);
        }
        glPopMatrix();

        // Head — sphere on top of the visible half
        glColor(head_color_);
        glPushMatrix();
        {
            glTranslatef(pos[0], pos[1] + visible_body_top_ + head_radius_, pos[2]);
            GLUquadric *q = gluNewQuadric();
            gluQuadricNormals(q, GLU_SMOOTH);
            gluSphere(q, head_radius_, 16, 16);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }
};

} // namespace soccernoid
