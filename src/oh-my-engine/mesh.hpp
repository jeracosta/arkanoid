#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GL/gl.h>
#include <GL/glext.h>

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

    // #region static asserts for Vertex layout

    static_assert(sizeof(Vertex) == 8 * sizeof(float),
                  "Mesh::Vertex must be tightly packed (8 floats) for GL interleaved VBO upload");
    static_assert(offsetof(Vertex, position) == 0 * sizeof(float),
                  "Mesh::Vertex.position must be at offset 0");
    static_assert(offsetof(Vertex, normal) == 3 * sizeof(float),
                  "Mesh::Vertex.normal must be at offset 3 floats");
    static_assert(offsetof(Vertex, texture_coords) == 6 * sizeof(float),
                  "Mesh::Vertex.texture_coords must be at offset 6 floats");

    // #endregion

    struct Primitive
    {
        std::vector<Vertex>   vertices;
        std::vector<unsigned> indices;
    };

    struct Node
    {
        Primitive         primitive;
        std::vector<Node> children = {};

        Node(auto primitive, std::vector<Node> children = {})
            : primitive(std::move(primitive)),
              children(std::move(children))
        {
        }

        GLuint
        gl_vertex_buffer_id() const noexcept
        {
            return gl_vertex_buffer_id_;
        }

        GLuint
        gl_element_buffer_id() const noexcept
        {
            return gl_element_buffer_id_;
        }

      private:
        GLuint gl_vertex_buffer_id_  = 0;
        GLuint gl_element_buffer_id_ = 0;

        friend class Mesh;
    };

    Mesh() = delete;

    ~Mesh();

    // clang-format off
    Mesh(const Mesh &)            = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&)                 = delete;
    Mesh &operator=(Mesh &&)      = delete;
    // clang-format on

    static std::shared_ptr<Mesh>
    load(const std::filesystem::path &path);

    static std::shared_ptr<Mesh>
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

    const Node &
    root() const noexcept
    {
        return root_;
    }

    void
    recenter(Vec3f new_origin = { 0.0f });

  private:
    Mesh(Node root);

    Node    root_;
    GLsizei index_count_;
};

} // namespace ome
