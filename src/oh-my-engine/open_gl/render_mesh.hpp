#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <GL/gl.h>
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/open_gl/matrix_guard.hpp"
#include "oh-my-engine/texture.hpp"
#include "oh-my-engine/transform.hpp"

namespace ome::open_gl {

struct MeshRenderTask
{
    const Mesh               &mesh;
    Transform                 transform;
    std::optional<Sprite>     sprite;
    std::optional<ome::Color> modulate;
    GLenum                    texture_env_mode = GL_MODULATE;

    void
    operator()() const
    {
        [[unlikely]]
        if (mesh.index_count() == 0)
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

        const bool use_texture = sprite.has_value() && static_cast<bool>(sprite->texture);

        glm::mat4 M
            = glm::translate(
                  glm::mat4(1.0f),
                  glm::vec3(transform.position[0], transform.position[1], transform.position[2]))
              * transform.orientation.matrix()
              * glm::scale(glm::mat4(1.0f),
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

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        {
            std::vector<const Mesh::Node *> stack = { &mesh.root() };
            while (!stack.empty())
            {
                const auto *node = stack.back();
                stack.pop_back();

                if (!node->primitive.vertices.empty())
                {
                    glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Vertex),
                                    node->primitive.vertices.data());
                    glNormalPointer(GL_FLOAT, sizeof(Mesh::Vertex),
                                    &node->primitive.vertices[0].normal);
                    glTexCoordPointer(2, GL_FLOAT, sizeof(Mesh::Vertex),
                                      &node->primitive.vertices[0].texture_coords);

                    glDrawElements(GL_TRIANGLES,
                                   static_cast<GLsizei>(node->primitive.indices.size()),
                                   GL_UNSIGNED_INT,
                                   node->primitive.indices.data());
                }

                for (const auto &child : node->children)
                {
                    stack.push_back(&child);
                }
            }
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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

} // namespace ome::open_gl
