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

Mesh::Mesh(Node root)
    : root_(std::move(root)),
      index_count_(0),
      vertex_count_(0)
{
    for (auto &node : nodes_view_(root_))
    {
        index_count_ += node.primitive.indices.size();
        vertex_count_ += node.primitive.vertices.size();
    }
}

Mesh::~Mesh() = default;

static std::pair<Vec3f, Vec3f>
aabb_(const Mesh &mesh)
{
    Vec3f min{ std::numeric_limits<float>::infinity() };
    Vec3f max{ -std::numeric_limits<float>::infinity() };

    for (const auto &node : nodes_view_(mesh.root()))
    {
        for (const auto &vertice : node.primitive.vertices)
        {
            min = math::zip_transform(std::ranges::min, min, vertice.position);
            max = math::zip_transform(std::ranges::max, max, vertice.position);
        }
    }

    return { min, max };
}

void
Mesh::recenter(Vec3f new_origin)
{
    if (vertex_count_ == 0)
    {
        return;
    }

    const Vec3f offset = new_origin - center();

    for (auto &node : nodes_view_(root_))
    {
        for (auto &v : node.primitive.vertices)
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

    const auto [min, max] = aabb_(*this);
    return max - min;
}

Vec3f
Mesh::center() const
{
    if (vertex_count_ == 0)
    {
        return { 0.0f };
    }

    const auto [min, max] = aabb_(*this);
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

    for (auto &node : nodes_view_(root_))
    {
        for (auto &vertice : node.primitive.vertices)
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
        Node root{ Primitive{
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
        }};

        return std::shared_ptr<const Mesh>(new Mesh(std::move(root)));
    }();

    return quad;
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

    Node root{ Primitive{} };

    auto process =
        [&](const aiNode *a_node, const aiMatrix4x4 &parent_tfm, Node &out_node, auto &self) -> void
    {
        const aiMatrix4x4 transform = parent_tfm * a_node->mTransformation;

        for (unsigned i = 0; i < a_node->mNumMeshes; ++i)
        {
            const aiMesh *amesh = scene->mMeshes[a_node->mMeshes[i]];

            Node mesh_node{ Primitive{} };
            mesh_node.primitive.material_index = static_cast<std::size_t>(amesh->mMaterialIndex);

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

                mesh_node.primitive.vertices.push_back(
                    Mesh::Vertex{ .position       = { tp.x, tp.y, tp.z },
                                  .normal         = { tn.x, tn.y, tn.z },
                                  .texture_coords = uv });
            }

            for (unsigned f = 0; f < amesh->mNumFaces; ++f)
            {
                const aiFace &face = amesh->mFaces[f];

                if (face.mNumIndices != 3U)
                {
                    throw std::runtime_error(std::format(
                        "Non-triangle face in mesh `{}` after triangulation.", path.string()));
                }

                mesh_node.primitive.indices.push_back(face.mIndices[0]);
                mesh_node.primitive.indices.push_back(face.mIndices[1]);
                mesh_node.primitive.indices.push_back(face.mIndices[2]);
            }

            out_node.children.push_back(std::move(mesh_node));
        }

        for (unsigned i = 0; i < a_node->mNumChildren; ++i)
        {
            Node child_node{ Primitive{} };
            self(a_node->mChildren[i], transform, child_node, self);

            if (!child_node.primitive.vertices.empty() || !child_node.children.empty())
            {
                out_node.children.push_back(std::move(child_node));
            }
        }
    };

    aiMatrix4x4 identity;
    process(scene->mRootNode, identity, root, process);

    if (root.primitive.vertices.empty() && root.children.empty())
    {
        throw std::runtime_error(
            std::format("No meshes found in `{}` after scene-graph traversal.", path.string()));
    }

    return std::shared_ptr<Mesh>(new Mesh(std::move(root)));
}

} // namespace ome
