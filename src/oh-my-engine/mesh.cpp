#include "oh-my-engine/mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/matrix3x3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <format>
#include <generator>
#include <limits>
#include <ranges>
#include <stdexcept>

#include "oh-my-engine/camera.hpp"

namespace ome {

template <typename NodeT>
static std::generator<NodeT &>
nodes_view_(NodeT &root)
{
    co_yield root;
    for (auto &child : root.children)
    {
        for (auto &node : nodes_view_(child))
        {
            co_yield node;
        }
    }
}

static std::pair<Vec3f, Vec3f>
aabb_(const std::vector<Mesh::Surface> &primitives)
{
    Vec3f min{ std::numeric_limits<float>::infinity() };
    Vec3f max{ -std::numeric_limits<float>::infinity() };

    for (const auto &primitive : primitives)
    {
        for (const auto &vertex : primitive.vertices)
        {
            min = math::zip_transform(std::ranges::min, min, vertex.position);
            max = math::zip_transform(std::ranges::max, max, vertex.position);
        }
    }

    return { min, max };
}

Mesh::Mesh(std::vector<Surface> primitives, Node root)
    : primitives_(std::move(primitives)),
      root_(std::move(root)),
      index_count_(0),
      vertex_count_(0)
{
    for (const auto &primitive : primitives_)
    {
        index_count_ += static_cast<GLsizei>(primitive.indices.size());
        vertex_count_ += static_cast<GLsizei>(primitive.vertices.size());
    }
}

Mesh::~Mesh() = default;

void
Mesh::recenter(Vec3f new_origin)
{
    if (vertex_count_ == 0)
    {
        return;
    }

    const Vec3f offset = new_origin - center();

    for (auto &primitive : primitives_)
    {
        for (auto &v : primitive.vertices)
        {
            v.position += offset;
        }
    }
}

Vec3f
Mesh::size() const
{
    if (vertex_count_ == 0)
    {
        return { 0.0f };
    }

    const auto [min, max] = aabb_(primitives_);
    return max - min;
}

Vec3f
Mesh::center() const
{
    if (vertex_count_ == 0)
    {
        return { 0.0f };
    }

    const auto [min, max] = aabb_(primitives_);
    return (min + max) * 0.5f;
}

void
Mesh::resize(const Vec3f &new_size)
{
    if (vertex_count_ == 0)
    {
        return;
    }

    const Vec3f current = size();
    Vec3f       scale{ 1.0f };

    for (auto &&[c, n, s] : std::views::zip(current, new_size, scale))
    {
        if (std::abs(c) > 1e-8f)
        {
            s = n / c;
        }
    }

    for (auto &primitive : primitives_)
    {
        for (auto &vertice : primitive.vertices)
        {
            vertice.position *= scale;

            for (auto &&[component, axis_scale] : std::views::zip(vertice.normal, scale))
            {
                if (std::abs(axis_scale) > 1e-8f)
                {
                    component /= axis_scale;
                }
            }

            vertice.normal = normalized(vertice.normal);
        }
    }
}

std::shared_ptr<const Mesh>
Mesh::unit_quad()
{
    static const std::shared_ptr<const Mesh> quad = []()
    {
        std::vector<Surface> primitives;
        primitives.push_back(Surface{
            .vertices = {
                {
                    .position       = { -0.5f, -0.5f, 0.0f },
                    .normal         = { 0.0f, 0.0f, 1.0f },
                    .texture_coords = { 0.0f, 0.0f },
                },
                {
                    .position       = { 0.5f, -0.5f, 0.0f },
                    .normal         = { 0.0f, 0.0f, 1.0f },
                    .texture_coords = { 1.0f, 0.0f },
                },
                {
                    .position       = { 0.5f, 0.5f, 0.0f },
                    .normal         = { 0.0f, 0.0f, 1.0f },
                    .texture_coords = { 1.0f, 1.0f },
                },
                {
                    .position       = { -0.5f, 0.5f, 0.0f },
                    .normal         = { 0.0f, 0.0f, 1.0f },
                    .texture_coords = { 0.0f, 1.0f },
                },
            },
            .indices = { 0, 1, 2, 0, 2, 3 },
        });

        Node root{ std::vector<std::size_t>{ 0 } };
        return std::make_shared<Mesh>(std::move(primitives), std::move(root));
    }();

    return quad;
}

std::shared_ptr<Mesh>
Mesh::box(const Box &box, std::size_t subdivisions)
{
    auto make_surface = [&](const BoxFace &face) -> Surface
    {
        auto edge1 = face.corners[1] - face.corners[0];
        auto edge2 = face.corners[3] - face.corners[0];
        auto tex_u = norm(edge1);
        auto tex_v = norm(edge2);

        if (subdivisions <= 1)
        {
            return Surface{
                .vertices = {
                    { .position = face.corners[0], .normal = face.normal, .texture_coords = { 0.0f, 0.0f } },
                    { .position = face.corners[1], .normal = face.normal, .texture_coords = { tex_u, 0.0f } },
                    { .position = face.corners[2], .normal = face.normal, .texture_coords = { tex_u, tex_v } },
                    { .position = face.corners[3], .normal = face.normal, .texture_coords = { 0.0f, tex_v } },
                },
                .indices        = { 0, 1, 2, 0, 2, 3 },
                .primitive_type = GL_TRIANGLES,
            };
        }

        auto const &c0 = face.corners[0];
        auto const &c1 = face.corners[1];
        auto const &c2 = face.corners[2];
        auto const &c3 = face.corners[3];

        std::vector<Vertex>     vertices;
        std::vector<unsigned>   indices;
        float                   step = 1.0f / static_cast<float>(subdivisions);

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

        auto idx = [&](std::size_t x, std::size_t y) -> unsigned
        { return static_cast<unsigned>(y * (subdivisions + 1) + x); };

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

        return Surface{ .vertices = std::move(vertices),
                        .indices  = std::move(indices),
                        .primitive_type = GL_QUADS };
    };

    std::vector<Surface> surfaces;
    surfaces.reserve(6);

    auto f = faces_of(box);
    surfaces.push_back(make_surface(f.front));
    surfaces.push_back(make_surface(f.back));
    surfaces.push_back(make_surface(f.left));
    surfaces.push_back(make_surface(f.right));
    surfaces.push_back(make_surface(f.top));
    surfaces.push_back(make_surface(f.bottom));

    Node root{ { 0, 1, 2, 3, 4, 5 } };

    return std::make_shared<Mesh>(std::move(surfaces), std::move(root));
}

std::shared_ptr<Mesh>
Mesh::quad(Vec3f a, Vec3f b, Vec3f c, Vec3f d)
{
    Vec3f normal = math::normalized(Vec3f{ glm::cross(glm::vec3(b - a), glm::vec3(d - a)) });

    return std::make_shared<Mesh>(
        std::vector<Surface>{
            Surface{
                .vertices = {
                    { .position = a, .normal = normal, .texture_coords = { 0.0f, 0.0f } },
                    { .position = b, .normal = normal, .texture_coords = { 1.0f, 0.0f } },
                    { .position = c, .normal = normal, .texture_coords = { 1.0f, 1.0f } },
                    { .position = d, .normal = normal, .texture_coords = { 0.0f, 1.0f } },
                },
                .indices        = { 0, 1, 2, 0, 2, 3 },
                .primitive_type = GL_TRIANGLES,
            },
        },
        Node{ { 0 } });
}

std::shared_ptr<Mesh>
Mesh::billboard(Vec3f position, Vec2f size, const Camera &camera)
{
    Vec3f right = camera.right() * (size[0] * 0.5f);
    Vec3f up    = camera.up() * (size[1] * 0.5f);

    return quad(
        position - right - up,
        position + right - up,
        position + right + up,
        position - right + up);
}

std::shared_ptr<Mesh>
Mesh::pyramid(Vec3f apex, Vec3f direction, float height, Vec2f base_half_extents)
{
    direction = math::normalized(direction);

    Vec3f ref       = std::abs(direction[0]) > 0.9f ? Vec3f{ 0, 1, 0 } : Vec3f{ 1, 0, 0 };
    Vec3f right     = math::normalized(Vec3f{ glm::cross(glm::vec3(direction), glm::vec3(ref)) });
    Vec3f forward   = Vec3f{ glm::cross(glm::vec3(direction), glm::vec3(right)) };
    Vec3f base_ctr  = apex + direction * height;

    Vec3f c0 = base_ctr + right * base_half_extents[0] + forward * base_half_extents[1];
    Vec3f c1 = base_ctr - right * base_half_extents[0] + forward * base_half_extents[1];
    Vec3f c2 = base_ctr - right * base_half_extents[0] - forward * base_half_extents[1];
    Vec3f c3 = base_ctr + right * base_half_extents[0] - forward * base_half_extents[1];

    std::vector<Surface> surfaces;
    surfaces.reserve(5);

    auto make_side_surface = [&](Vec3f ca, Vec3f cb) -> Surface
    {
        Vec3f n = math::normalized(Vec3f{ glm::cross(glm::vec3(ca - apex), glm::vec3(cb - apex)) });
        return Surface{
            .vertices = {
                { .position = apex, .normal = n, .texture_coords = { 0.0f, 0.0f } },
                { .position = ca,   .normal = n, .texture_coords = { 1.0f, 0.0f } },
                { .position = cb,   .normal = n, .texture_coords = { 0.0f, 1.0f } },
            },
            .indices        = { 0, 1, 2 },
            .primitive_type = GL_TRIANGLES,
            .material_index = 0,
        };
    };

    surfaces.push_back(make_side_surface(c0, c1));
    surfaces.push_back(make_side_surface(c1, c2));
    surfaces.push_back(make_side_surface(c2, c3));
    surfaces.push_back(make_side_surface(c3, c0));

    surfaces.push_back(Surface{
        .vertices = {
            { .position = c0, .normal = -direction, .texture_coords = { 0.0f, 0.0f } },
            { .position = c1, .normal = -direction, .texture_coords = { 1.0f, 0.0f } },
            { .position = c2, .normal = -direction, .texture_coords = { 1.0f, 1.0f } },
            { .position = c3, .normal = -direction, .texture_coords = { 0.0f, 1.0f } },
        },
        .indices        = { 0, 1, 2, 0, 2, 3 },
        .primitive_type = GL_TRIANGLES,
        .material_index = 1,
    });

    Node root{ { 0, 1, 2, 3, 4 } };

    return std::make_shared<Mesh>(std::move(surfaces), std::move(root));
}

std::shared_ptr<Mesh>
Mesh::load(const std::filesystem::path &path)
{
    Assimp::Importer importer;

    constexpr unsigned import_flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                      | aiProcess_ImproveCacheLocality
                                      | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;

    const aiScene *scene = importer.ReadFile(path.string(), import_flags);

    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0u
        || scene->mRootNode == nullptr)
    {
        throw std::runtime_error(
            std::format("Failed to load mesh `{}`: {}", path.string(), importer.GetErrorString()));
    }

    if (scene->mNumMeshes == 0)
    {
        throw std::runtime_error(std::format("Mesh `{}` contains no geometry.", path.string()));
    }

    std::vector<Surface> primitives;
    Node                 root{};

    auto convert_mesh = [&](const aiMesh *amesh, const aiMatrix4x4 &transform) -> Surface
    {
        Surface primitive;
        primitive.material_index = static_cast<std::size_t>(amesh->mMaterialIndex);

        for (unsigned v = 0; v < amesh->mNumVertices; ++v)
        {
            const aiVector3D tp = transform * amesh->mVertices[v];

            aiVector3D tn{ 0.0f, 1.0f, 0.0f };
            if (amesh->HasNormals())
            {
                aiMatrix3x3 normal_tfm(transform);
                normal_tfm.Inverse();
                normal_tfm.Transpose();
                tn = (normal_tfm * amesh->mNormals[v]).Normalize();
            }

            Vec2f uv{ 0.0f, 0.0f };
            if (amesh->HasTextureCoords(0))
            {
                const aiVector3D &ai_uv = amesh->mTextureCoords[0][v];
                uv                      = Vec2f{ ai_uv.x, ai_uv.y };
            }

            primitive.vertices.push_back(Mesh::Vertex{
                .position       = { tp.x, tp.y, tp.z },
                .normal         = { tn.x, tn.y, tn.z },
                .texture_coords = uv,
            });
        }

        for (unsigned f = 0; f < amesh->mNumFaces; ++f)
        {
            const aiFace &face = amesh->mFaces[f];

            if (face.mNumIndices != 3U)
            {
                throw std::runtime_error(std::format(
                    "Non-triangle face in mesh `{}` after triangulation.", path.string()));
            }

            primitive.indices.push_back(face.mIndices[0]);
            primitive.indices.push_back(face.mIndices[1]);
            primitive.indices.push_back(face.mIndices[2]);
        }

        return primitive;
    };

    auto process =
        [&](const aiNode *a_node, const aiMatrix4x4 &parent_tfm, Node &out_node, auto &self) -> void
    {
        const aiMatrix4x4 transform = parent_tfm * a_node->mTransformation;

        for (unsigned i = 0; i < a_node->mNumMeshes; ++i)
        {
            const aiMesh     *amesh           = scene->mMeshes[a_node->mMeshes[i]];
            const std::size_t primitive_index = primitives.size();
            primitives.push_back(convert_mesh(amesh, transform));
            out_node.surface_indices.push_back(primitive_index);
        }

        for (unsigned i = 0; i < a_node->mNumChildren; ++i)
        {
            Node child_node{};
            self(a_node->mChildren[i], transform, child_node, self);

            if (!child_node.surface_indices.empty() || !child_node.children.empty())
            {
                out_node.children.push_back(std::move(child_node));
            }
        }
    };

    aiMatrix4x4 identity;
    process(scene->mRootNode, identity, root, process);

    if (primitives.empty() || (root.surface_indices.empty() && root.children.empty()))
    {
        throw std::runtime_error(
            std::format("No meshes found in `{}` after scene-graph traversal.", path.string()));
    }

    return std::shared_ptr<Mesh>(new Mesh(std::move(primitives), std::move(root)));
}

} // namespace ome
