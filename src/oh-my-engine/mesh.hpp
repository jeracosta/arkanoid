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

namespace ome {

namespace open_gl {
struct MeshRenderTask;
}

class Mesh
{
    friend struct open_gl::MeshRenderTask;

  public:
    Mesh() = delete;

    ~Mesh();

    Mesh(const Mesh &)            = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&)                 = delete;
    Mesh &operator=(Mesh &&)     = delete;

    static std::shared_ptr<Mesh>
    load(const std::filesystem::path &path);

    void
    recenter_to_origin();

    void
    normalize_to_max_extent(float max_extent);

    bool
    has_uv() const noexcept
    {
        return has_uv_;
    }

    GLsizei
    index_count() const noexcept
    {
        return index_count_;
    }

  private:
    Mesh(std::vector<float> interleaved, std::vector<unsigned> indices, bool has_uv);

    void
    reupload_();

    std::vector<float>    interleaved_;
    std::vector<unsigned> indices_;
    bool                  has_uv_;
    GLsizei               stride_bytes_;
    GLsizei               index_count_;

    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};

}