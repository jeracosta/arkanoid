#pragma once

#include "oh-my-engine/node.hpp"
#include "oh-my-engine/open_gl/render_box.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

class SkyboxNode : public ome::Node
{
  public:
    SkyboxNode()
        : Node(),
          render_task_(make_render_task_())
    {
    }

    void
    on_tick_() override
    {
        auto      position = game()->camera.position();
        GLboolean fog_was  = glIsEnabled(GL_FOG);

        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);

        glDepthMask(GL_FALSE);
        glDisable(GL_FOG);

        render_task_();

        if (fog_was)
        {
            glEnable(GL_FOG);
        }

        glDepthMask(GL_TRUE);
        glPopMatrix();
    }

  private:
    static constexpr float side_ = 2 * fog.end;

    static const TexturePalette::SkyboxFaces &
    active_skybox_()
    {
        return textures.skybox.blink;
    }

    using Sprite = ome::open_gl::BoxRenderTask::Sprite;

    static Sprite
    make_sprite_(const TexturePalette::Item &item)
    {
        return { .texture   = static_cast<std::shared_ptr<ome::Texture>>(item),
                 .uv_region = { { 0.0f, 0.0f }, { 1.0f, 1.0f } } };
    }

    static ome::open_gl::BoxRenderTask
    make_render_task_()
    {
        const auto &skybox = active_skybox_();

        return { .world_region = ome::math::Box<3>(side_),
                 .sprites      = {
                    .front  = make_sprite_(skybox.front),
                    .back   = make_sprite_(skybox.back),
                    .left   = make_sprite_(skybox.left),
                    .right  = make_sprite_(skybox.right),
                    .top    = make_sprite_(skybox.top),
                    .bottom = make_sprite_(skybox.bottom),
                 } };
    }

    ome::open_gl::BoxRenderTask render_task_;
};

} // namespace soccernoid
