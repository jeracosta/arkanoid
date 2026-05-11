#include "oh-my-engine/color.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class TerrainNode : public ome::HitboxNode
{
  public:
    // Futsal: penalty point (origin) to goal line (-6m), full court width (20m centered at origin)
    TerrainNode()
        : HitboxNode({ .min = { -10.0f, 0.0f, -6.0f }, .max = { 10.0f, 0.0f, 0.0f } })
    {
    }

    void
    on_tick_() override
    {
        render_box_();
        render_arco_();
    }

  private:
    void
    render_box_()
    {
        const auto corners = hitbox_world().corners();

        // corners[0] = (-10, 0, -6)  back-left
        // corners[1] = ( 10, 0, -6)  back-right
        // corners[2] = ( 10, 0,  0)  front-right
        // corners[3] = (-10, 0,  0)  front-left

        float depth = -terrain_box_depth;

        glColor(colors.grass);
        glBegin(GL_QUADS);
        {
            for (const auto &corner : corners)
            {
                glVertex3f(corner[0], corner[1], corner[2]);
            }
        }
        glEnd();

        glColor(colors.dirt);
        glBegin(GL_QUADS);
        {
            // Front (z = 0)
            glVertex3f(corners[3][0], corners[3][1], corners[3][2]);
            glVertex3f(corners[2][0], corners[2][1], corners[2][2]);
            glVertex3f(corners[2][0], depth, corners[2][2]);
            glVertex3f(corners[3][0], depth, corners[3][2]);

            // Back (z = -6)
            glVertex3f(corners[1][0], corners[1][1], corners[1][2]);
            glVertex3f(corners[0][0], corners[0][1], corners[0][2]);
            glVertex3f(corners[0][0], depth, corners[0][2]);
            glVertex3f(corners[1][0], depth, corners[1][2]);

            // Left (x = -10)
            glVertex3f(corners[0][0], corners[0][1], corners[0][2]);
            glVertex3f(corners[3][0], corners[3][1], corners[3][2]);
            glVertex3f(corners[3][0], depth, corners[3][2]);
            glVertex3f(corners[0][0], depth, corners[0][2]);

            // Right (x = 10)
            glVertex3f(corners[2][0], corners[2][1], corners[2][2]);
            glVertex3f(corners[1][0], corners[1][1], corners[1][2]);
            glVertex3f(corners[1][0], depth, corners[1][2]);
            glVertex3f(corners[2][0], depth, corners[2][2]);
        }
        glEnd();
    }

    void
    render_arco_()
    {
        const auto corners = hitbox_world().corners();

        // FIFA futsal goal: 3m wide × 2m tall
        constexpr float goal_height     = 2.0f;
        constexpr float goal_half_width = 1.5f;

        const float z = corners[0][2]; // near edge of the field (goal line)
        const float y = corners[0][1];

        const float center_x = (corners[0][0] + corners[1][0]) * 0.5f;

        glBegin(GL_QUADS);
        {
            glColor(colors.goal);
            glVertex3f(center_x + goal_half_width, y, z);
            glVertex3f(center_x + goal_half_width, y + goal_height, z);
            glVertex3f(center_x - goal_half_width, y + goal_height, z);
            glVertex3f(center_x - goal_half_width, y, z);
        }
        glEnd();
    }
};

} // namespace soccernoid
