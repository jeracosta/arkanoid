#include "oh-my-engine/nodes/hitbox_node.hpp"

namespace soccernoid {

class TerrainNode : public ome::HitboxNode
{
  public:
    TerrainNode()
        : HitboxNode({ .min = { -1.0f, 0.0f, -1.0f }, .max = { 1.0f, 0.0f, 1.0f } })
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

        constexpr auto height = 0.2f;

        const float z = corners[0][2]; // near edge of the field
        const float y = corners[0][1];

        const float center_x   = (corners[0][0] + corners[1][0]) * 0.5f;
        const float half_width = (corners[1][0] - corners[0][0]) * 0.125f; // 1/4 of field width

        glBegin(GL_QUADS);
        {
            glColor(ome::Color::rgb(0.9, 0.95, 0.85));
            glVertex3f(center_x + half_width, y, z);
            glVertex3f(center_x + half_width, y + height, z);
            glVertex3f(center_x - half_width, y + height, z);
            glVertex3f(center_x - half_width, y, z);
        }
        glEnd();
    }
};

} // namespace soccernoid
