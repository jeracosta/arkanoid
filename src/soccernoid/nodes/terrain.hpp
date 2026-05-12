#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
#include "oh-my-engine/texture.hpp"
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

        constexpr float depth = -fog.end;

        glColor(colors.grass);
        glBegin(GL_QUADS);
        {
            for (const auto &corner : corners)
            {
                glVertex3f(corner[0], corner[1], corner[2]);
            }
        }
        glEnd();

        glEnable(GL_TEXTURE_2D);
        ome::open_gl::glBindTexture(textures.dirt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        {
            const float side_height    = -depth;
            const float left_rgt_width = corners[3][2] - corners[0][2];
            const float frnt_back_wdth = corners[1][0] - corners[0][0];

            glBegin(GL_QUADS);
            {
                // Left
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(corners[0][0], corners[0][1], corners[0][2]);
                glTexCoord2f(left_rgt_width, 0.0f);
                glVertex3f(corners[3][0], corners[3][1], corners[3][2]);
                glTexCoord2f(left_rgt_width, side_height);
                glVertex3f(corners[3][0], depth, corners[3][2]);
                glTexCoord2f(0.0f, side_height);
                glVertex3f(corners[0][0], depth, corners[0][2]);

                // Right
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(corners[2][0], corners[2][1], corners[2][2]);
                glTexCoord2f(left_rgt_width, 0.0f);
                glVertex3f(corners[1][0], corners[1][1], corners[1][2]);
                glTexCoord2f(left_rgt_width, side_height);
                glVertex3f(corners[1][0], depth, corners[1][2]);
                glTexCoord2f(0.0f, side_height);
                glVertex3f(corners[2][0], depth, corners[2][2]);

                // Front
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(corners[3][0], corners[3][1], corners[3][2]);
                glTexCoord2f(frnt_back_wdth, 0.0f);
                glVertex3f(corners[2][0], corners[2][1], corners[2][2]);
                glTexCoord2f(frnt_back_wdth, side_height);
                glVertex3f(corners[2][0], depth, corners[2][2]);
                glTexCoord2f(0.0f, side_height);
                glVertex3f(corners[3][0], depth, corners[3][2]);

                // Back
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(corners[1][0], corners[1][1], corners[1][2]);
                glTexCoord2f(frnt_back_wdth, 0.0f);
                glVertex3f(corners[0][0], corners[0][1], corners[0][2]);
                glTexCoord2f(frnt_back_wdth, side_height);
                glVertex3f(corners[0][0], depth, corners[0][2]);
                glTexCoord2f(0.0f, side_height);
                glVertex3f(corners[1][0], depth, corners[1][2]);
            }
            glEnd();
        }
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_TEXTURE_2D);
        ome::open_gl::glBindTexture(*ome::Texture::placeholder());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        {
            const float floor_width = corners[1][0] - corners[0][0];
            const float floor_depth = corners[2][2] - corners[0][2];

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(corners[0][0], depth, corners[0][2]);

            glTexCoord2f(floor_width, 0.0f);
            glVertex3f(corners[1][0], depth, corners[1][2]);

            glTexCoord2f(floor_width, floor_depth);
            glVertex3f(corners[2][0], depth, corners[2][2]);

            glTexCoord2f(0.0f, floor_depth);
            glVertex3f(corners[3][0], depth, corners[3][2]);
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);
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
