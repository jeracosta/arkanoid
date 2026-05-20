#pragma once

#include <GL/gl.h>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

class Mesh
{
  public:
    struct Vertex
    {
        Vec3f position;
        Vec3f normal;
        Vec2f texture_coords;
    };

    static_assert(sizeof(Vertex) == 8 * sizeof(float),
                  "Mesh::Vertex must be tightly packed (8 floats) for interleaved array stride");
    static_assert(offsetof(Vertex, position) == 0 * sizeof(float),
                  "Mesh::Vertex.position must be at offset 0");
    static_assert(offsetof(Vertex, normal) == 3 * sizeof(float),
                  "Mesh::Vertex.normal must be at offset 3 floats");
    static_assert(offsetof(Vertex, texture_coords) == 6 * sizeof(float),
                  "Mesh::Vertex.texture_coords must be at offset 6 floats");

    struct Primitive
    {
        std::vector<Vertex>        vertices;
        std::vector<unsigned int>   indices;
        std::optional<std::size_t> material_index = std::nullopt;
    };

    struct Node
    {
        Primitive         primitive;
        std::vector<Node> children = {};

        Node(Primitive primitive, std::vector<Node> children = {})
            : primitive(std::move(primitive)),
              children(std::move(children))
        {
        }
    };

    Mesh() = delete;
    ~Mesh();

    Mesh(const Mesh &) = delete;

    Mesh &
    operator=(const Mesh &)
        = delete;

    Mesh(Mesh &&) = delete;

    Mesh &
    operator=(Mesh &&)
        = delete;

    static std::shared_ptr<Mesh>
    load(const std::filesystem::path &path);

    static std::shared_ptr<const Mesh>
    unit_quad();

    Vec3f
    size() const;

    Vec3f
    center() const;

    void
    resize(const Vec3f &new_size);

    GLsizei
    index_count() const noexcept
    {
        return index_count_;
    }

    GLsizei
    vertex_count() const noexcept
    {
        return vertex_count_;
    }

    const Node &
    root() const noexcept
    {
        return root_;
    }

    void
    recenter(Vec3f new_origin = { 0.0f });

  private:
    explicit Mesh(Node root);

    Node    root_;
    GLsizei index_count_;
    GLsizei vertex_count_;
};

} // namespace ome
