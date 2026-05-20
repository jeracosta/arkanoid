#include "oh-my-engine/render_frame.hpp"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <span>

#include "oh-my-engine/draw_command.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/open_gl/matrix_guard.hpp"
#include "oh-my-engine/skybox.hpp"

namespace ome::open_gl {

static void
render_(const Mesh::Node         &mesh_node,
        std::span<const Material> materials,
        const Transform          &transform = {})
{
    auto model_guard = MatrixGuard(GL_MODELVIEW);

    glMultMatrixf(glm::value_ptr(static_cast<glm::mat4>(transform)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    {
        std::vector<const Mesh::Node *> stack = { &mesh_node };
        while (!stack.empty())
        {
            const auto *node = stack.back();
            stack.pop_back();

            if (!node->primitive.vertices.empty())
            {

                bool has_material = node->primitive.material_index.has_value()
                                    && node->primitive.material_index.value() < materials.size();

                auto material
                    = has_material ? materials[*node->primitive.material_index] : Material{};

                bind(material);

                glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Vertex), node->primitive.vertices.data());
                glNormalPointer(
                    GL_FLOAT, sizeof(Mesh::Vertex), &node->primitive.vertices[0].normal);
                glTexCoordPointer(
                    2, GL_FLOAT, sizeof(Mesh::Vertex), &node->primitive.vertices[0].texture_coords);

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
}

void
render(const RenderFrame &frame)
{
    [[likely]] if (frame.skybox.has_value())
    {
        open_gl::render(*frame.skybox);
    }

    auto renders_first = [](const DrawCommand &lhs, const DrawCommand &rhs)
    {
        auto lhs_layer = static_cast<unsigned>(lhs.layer);
        auto rhs_layer = static_cast<unsigned>(rhs.layer);
        return lhs_layer < rhs_layer;
    };

    auto sorted_draw_commands = frame.draw_commands; // unnecesary copy?
    std::ranges::sort(sorted_draw_commands, renders_first);

    for (const auto &draw_command : sorted_draw_commands)
    {
        render_(draw_command.mesh->root(), draw_command.materials, draw_command.transform);
    }
}

} // namespace ome::open_gl
