#include "oh-my-engine/draw_command.hpp"

#include "oh-my-engine/math/interval.hpp"

namespace ome {

DrawCommand
DrawCommand::box(const Box                  &box,
                 std::vector<Material>       materials,
                 const BoxFaces<std::size_t> material_indices,
                 std::size_t                 subdivisions)
{
    auto make_surface = [&](const BoxFace &face, std::size_t material_index) -> Mesh::Surface
    {
        auto edge1 = face.corners[1] - face.corners[0];
        auto edge2 = face.corners[3] - face.corners[0];
        auto tex_u = norm(edge1);
        auto tex_v = norm(edge2);

        if (subdivisions <= 1)
        {
            return Mesh::Surface{
                .vertices = {
                    {
                        .position       = face.corners[0],
                        .normal         = face.normal,
                        .texture_coords = { 0.0f, 0.0f },
                    },
                    {
                        .position       = face.corners[1],
                        .normal         = face.normal,
                        .texture_coords = { tex_u, 0.0f },
                    },
                    {
                        .position       = face.corners[2],
                        .normal         = face.normal,
                        .texture_coords = { tex_u, tex_v },
                    },
                    {
                        .position       = face.corners[3],
                        .normal         = face.normal,
                        .texture_coords = { 0.0f, tex_v },
                    },
                },

                .indices = { 0, 1, 2, 3 },

                .primitive_type = GL_QUADS,

                .material_index = material_index
            };
        }

        auto const &c0 = face.corners[0];
        auto const &c1 = face.corners[1];
        auto const &c2 = face.corners[2];
        auto const &c3 = face.corners[3];

        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        float                     step = 1.0f / static_cast<float>(subdivisions);

        for (std::size_t j = 0; j <= subdivisions; ++j)
        {
            for (std::size_t i = 0; i <= subdivisions; ++i)
            {
                float u = static_cast<float>(i) * step;
                float v = static_cast<float>(j) * step;

                auto p = (1.0f - u) * (1.0f - v) * c0 + u * (1.0f - v) * c1 + u * v * c2
                         + (1.0f - u) * v * c3;

                vertices.push_back({
                    .position       = p,
                    .normal         = face.normal,
                    .texture_coords = { u * tex_u, v * tex_v },
                });
            }
        }

        auto idx = [&](std::size_t x, std::size_t y) -> unsigned int
        { return static_cast<unsigned int>(y * (subdivisions + 1) + x); };

        for (std::size_t j = 0; j < subdivisions; ++j)
        {
            for (std::size_t i = 0; i < subdivisions; ++i)
            {
                indices.push_back(idx(i, j));
                indices.push_back(idx(i + 1, j));
                indices.push_back(idx(i + 1, j + 1));
                indices.push_back(idx(i, j + 1));
            }
        }

        return Mesh::Surface{ .vertices       = std::move(vertices),
                              .indices        = std::move(indices),
                              .primitive_type = GL_QUADS,
                              .material_index = material_index };
    };

    std::vector<Mesh::Surface> surfaces;
    surfaces.reserve(6);

    auto faces = faces_of(box);

    surfaces.push_back(make_surface(faces.front, material_indices.front));
    surfaces.push_back(make_surface(faces.back, material_indices.back));
    surfaces.push_back(make_surface(faces.left, material_indices.left));
    surfaces.push_back(make_surface(faces.right, material_indices.right));
    surfaces.push_back(make_surface(faces.top, material_indices.top));
    surfaces.push_back(make_surface(faces.bottom, material_indices.bottom));

    // Build mesh (single-node flat mesh)
    Mesh::Node root{};
    for (std::size_t i = 0; i < surfaces.size(); ++i)
    {
        root.surface_indices.push_back(i);
    }

    auto mesh = std::make_shared<Mesh>(std::move(surfaces), std::move(root));

    return DrawCommand{ .mesh      = std::move(mesh),
                        .materials = materials,
                        .transform = Transform{},
                        .layer     = DrawCommand::Layer::Opaque };
}

} // namespace ome
