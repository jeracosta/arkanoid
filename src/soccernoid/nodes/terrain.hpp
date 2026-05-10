#include "oh-my-engine/nodes/hitbox_node.hpp"

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
        render_cancha_();
        render_arco_();
    }

  private:
    void
    render_cancha_()
    {
        glBegin(GL_QUADS);
        {
            glColor(ome::Color::rgb(0.1, 0.8, 0.1));
            for (const auto &corner : hitbox_world().corners())
            {
                glVertex3f(corner[0], corner[1], corner[2]);
            }
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
            glColor(ome::Color::rgb(0.9, 0.95, 0.85));
            glVertex3f(center_x + goal_half_width, y, z);
            glVertex3f(center_x + goal_half_width, y + goal_height, z);
            glVertex3f(center_x - goal_half_width, y + goal_height, z);
            glVertex3f(center_x - goal_half_width, y, z);
        }
        glEnd();
    }
};

} // namespace soccernoid
