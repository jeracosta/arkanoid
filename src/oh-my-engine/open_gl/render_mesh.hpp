#pragma once

#include <cstdint>
#include <optional>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/open_gl/matrix_guard.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome::open_gl {

struct MeshRenderTask
{
    const Mesh               &mesh;
    TransformComponent        transform;
    std::optional<Sprite>     sprite;
    std::optional<ome::Color> modulate;
    GLenum                    texture_env_mode = GL_MODULATE;

    void
    operator()() const
    {
        if (mesh.index_count() <= 0)
        {
            return;
        }

        const GLint prev_env_mode = []
        {
            GLint mode = GL_MODULATE;
            glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);
            return mode;
        }();

        const GLboolean prev_texture_2d = glIsEnabled(GL_TEXTURE_2D);

        const bool use_texture
            = sprite.has_value() && static_cast<bool>(sprite->texture) && mesh.has_uv();

        glm::mat4 M = glm::translate(
                          glm::mat4(1.0f),
                          glm::vec3(transform.position[0], transform.position[1], transform.position[2]))
                      * transform.orientation.matrix()
                      * glm::scale(
                          glm::mat4(1.0f),
                          glm::vec3(transform.scale[0], transform.scale[1], transform.scale[2]));

        auto model_guard = MatrixGuard(GL_MODELVIEW);
        glMultMatrixf(glm::value_ptr(M));

        if (use_texture)
        {
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, static_cast<GLint>(texture_env_mode));
            glBindTexture(*sprite->texture);

            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
            glLoadIdentity();
            const auto &uv0 = sprite->uv_region.min();
            const auto &uv1 = sprite->uv_region.max();
            glTranslatef(uv0[0], uv0[1], 0.0f);
            glScalef(uv1[0] - uv0[0], uv1[1] - uv0[1], 1.0f);
            glMatrixMode(GL_MODELVIEW);

            glColor(modulate.value_or(ome::Color::white()));
        }
        else
        {
            if (prev_texture_2d)
            {
                glDisable(GL_TEXTURE_2D);
            }
            glColor(modulate.value_or(ome::Color::white()));
        }

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_);

        glVertexPointer(3, GL_FLOAT, mesh.stride_bytes_, nullptr);
        glNormalPointer(GL_FLOAT, mesh.stride_bytes_, reinterpret_cast<const void *>(static_cast<std::uintptr_t>(3 * sizeof(float))));

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        if (mesh.has_uv())
        {
            glTexCoordPointer(
                2,
                GL_FLOAT,
                mesh.stride_bytes_,
                reinterpret_cast<const void *>(static_cast<std::uintptr_t>(6 * sizeof(float))));
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        glDrawElements(GL_TRIANGLES, mesh.index_count_, GL_UNSIGNED_INT, nullptr);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        if (mesh.has_uv())
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (use_texture)
        {
            glMatrixMode(GL_TEXTURE);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, prev_env_mode);

        if (prev_texture_2d)
        {
            glEnable(GL_TEXTURE_2D);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
        }
    }
};

}
