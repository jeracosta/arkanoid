#include "oh-my-engine/render_frame.hpp"

#include <GL/gl.h>
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
render_(std::span<const Mesh::Vertex> vertices, std::span<const unsigned int> indices, GLenum mode)
{
    [[unlikely]]
    if (vertices.empty() || indices.empty())
    {
        return;
    }

    const auto *base   = reinterpret_cast<const std::byte *>(vertices.data());
    const auto  stride = sizeof(Mesh::Vertex);

    glVertexPointer(3, GL_FLOAT, stride, base + offsetof(Mesh::Vertex, position));
    glNormalPointer(GL_FLOAT, stride, base + offsetof(Mesh::Vertex, normal));
    glTexCoordPointer(2, GL_FLOAT, stride, base + offsetof(Mesh::Vertex, texture_coords));

    auto index_count = static_cast<GLsizei>(indices.size());

    glDrawElements(mode, index_count, GL_UNSIGNED_INT, indices.data());
}

static void
visit_dfs_(const Mesh::Node &node, auto &&visitor)
{
    visitor(node);

    for (const auto &child : node.children)
    {
        visit_dfs_(child, visitor);
    }
}

static bool
in_layer_order_(const DrawCommand &lhs, const DrawCommand &rhs)
{
    return static_cast<unsigned>(lhs.layer) < static_cast<unsigned>(rhs.layer);
};

static auto
sorted_(auto draw_commands)
{
    std::ranges::sort(draw_commands, in_layer_order_);
    return draw_commands;
}

struct LightBatch_
{
    static std::size_t
    capacity()
    {
        GLint value = 0;
        glGetIntegerv(GL_MAX_LIGHTS, &value);
        return static_cast<std::size_t>(value);
    }

    std::span<const Light> lights;
};

static auto
batch_view_(std::span<const Light> lights)
{
    using namespace std::views;

    const auto n = LightBatch_::capacity();

    auto indices = iota(std::size_t{ 0 }, (lights.size() + n - 1) / n);

    auto make_batch = [lights, n](std::size_t index) -> LightBatch_
    {
        std::size_t offset = index * n;
        std::size_t count  = std::min(n, lights.size() - offset);

        return { lights.subspan(offset, count) };
    };

    return indices | transform(make_batch);
}

struct RenderStateGuard_
{
    RenderStateGuard_()
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glEnable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);
        glEnable(GL_DEPTH_TEST);
    }

    ~RenderStateGuard_()
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_NORMALIZE);
        glDisable(GL_LIGHTING);
    }
};

void
render(const RenderFrame &frame)
{
    if (frame.skybox)
    {
        open_gl::render(*frame.skybox);
    }

    auto &lights        = frame.lights;
    auto  draw_commands = sorted_(frame.draw_commands);

    auto light_slots
        = std::views::iota(GLenum(GL_LIGHT0), GLenum(GL_LIGHT0 + LightBatch_::capacity()));

    RenderStateGuard_ render_state_guard;

    for (auto [light_batch_index, light_batch] : std::views::enumerate(batch_view_(lights)))
    {
        auto is_first_light_pass = light_batch_index == 0;

        for (auto slot : light_slots)
        {
            glDisable(slot);
        }

        for (auto [light, slot] : std::views::zip(light_batch.lights, light_slots))
        {
            bind(light, slot);
            glEnable(slot);
        }

        for (const auto &draw_command : draw_commands)
        {
            auto model_guard = MatrixGuard(GL_MODELVIEW);

            glMultMatrixf(glm::value_ptr(static_cast<glm::mat4>(draw_command.transform)));

            [[unlikely]]
            if (draw_command.mesh == nullptr)
            {
                continue;
            }

            auto &mesh      = *draw_command.mesh;
            auto &materials = draw_command.materials;

            auto mesh_node_visitor = [&](const Mesh::Node &node)
            {
                auto model_guard = MatrixGuard(GL_MODELVIEW);

                // TODO: Apply node transforms when implemented
                // glMultMatrixf(glm::value_ptr(static_cast<glm::mat4>(node.transform)));

                auto surfaces = node.surface_indices
                                | std::views::transform([&](auto i) -> const Mesh::Surface &
                { return mesh.surfaces()[i]; });

                for (auto &surface : surfaces)
                {
                    auto material = Material{}; // configured next

                    if (surface.material_index.has_value()
                        && *surface.material_index < materials.size())
                    {
                        material = materials[*surface.material_index];
                    }

                    if (!is_first_light_pass)
                    {
                        glDepthMask(GL_FALSE);
                        glDepthFunc(GL_LEQUAL);

                        material.color.ambient = Color::black();
                        material.blend_mode    = { GL_ONE, GL_ONE };
                    }
                    else
                    {

                        glDepthFunc(GL_LESS);
                        glDepthMask(GL_TRUE);
                    }

                    bind(material);

                    render_(surface.vertices, surface.indices, surface.primitive_type);
                }
            };

            visit_dfs_(mesh.root(), mesh_node_visitor);
        }
    }
}

} // namespace ome::open_gl
