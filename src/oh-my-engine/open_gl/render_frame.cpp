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
render_(const Mesh &mesh, const Mesh::Node &node, std::span<const Material> materials)
{
    auto model_guard = MatrixGuard(GL_MODELVIEW);
    // glMultMatrixf(glm::value_ptr(static_cast<glm::mat4>(node.transform)));

    for (std::size_t surface_index : node.surface_indices)
    {
        const auto &surface = mesh.surface(surface_index);

        if (surface.vertices.empty() || surface.indices.empty())
        {
            continue;
        }

        const Material *material = nullptr;
        if (surface.material_index.has_value() && *surface.material_index < materials.size())
        {
            material = &materials[*surface.material_index];
        }

        bind(material != nullptr ? *material : Material{});

        glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Vertex), surface.vertices.data());
        glNormalPointer(GL_FLOAT, sizeof(Mesh::Vertex), &surface.vertices.front().normal);
        glTexCoordPointer(
            2, GL_FLOAT, sizeof(Mesh::Vertex), &surface.vertices.front().texture_coords);

        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(surface.indices.size()),
                       GL_UNSIGNED_INT,
                       surface.indices.data());
    }

    for (const auto &child : node.children)
    {
        render_(mesh, child, materials);
    }
}

void
render(const RenderFrame &frame)
{
    [[likely]] if (frame.skybox.has_value())
    {
        open_gl::render(*frame.skybox);
    }

    auto renders_first = [](const DrawCommand &lhs, const DrawCommand &rhs)
    { return static_cast<unsigned>(lhs.layer) < static_cast<unsigned>(rhs.layer); };

    auto sorted_draw_commands = frame.draw_commands;
    std::ranges::sort(sorted_draw_commands, renders_first);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for (const auto &draw_command : sorted_draw_commands)
    {
        auto model_guard = MatrixGuard(GL_MODELVIEW);
        glMultMatrixf(glm::value_ptr(static_cast<glm::mat4>(draw_command.transform)));

        render_(*draw_command.mesh, draw_command.mesh->root(), draw_command.materials);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

} // namespace ome::open_gl
