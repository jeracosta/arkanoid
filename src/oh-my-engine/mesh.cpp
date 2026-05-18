#include "oh-my-engine/mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <format>
#include <limits>
#include <stdexcept>

namespace ome {

Mesh::~Mesh()
{
    if (vbo_ != 0)
    {
        glDeleteBuffers(1, &vbo_);
    }
    if (ebo_ != 0)
    {
        glDeleteBuffers(1, &ebo_);
    }
}

void
Mesh::reupload_()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(interleaved_.size() * sizeof(float)),
        interleaved_.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::Mesh(std::vector<float> interleaved, std::vector<unsigned> indices, bool has_uv)
    : interleaved_(std::move(interleaved)),
      indices_(std::move(indices)),
      has_uv_(has_uv),
      stride_bytes_(static_cast<GLsizei>((has_uv ? 8uz : 6uz) * sizeof(float))),
      index_count_(static_cast<GLsizei>(indices_.size()))
{
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(interleaved_.size() * sizeof(float)),
        interleaved_.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices_.size() * sizeof(unsigned)),
        indices_.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
Mesh::recenter_to_origin()
{
    if (interleaved_.empty())
    {
        return;
    }

    const std::size_t floats_per_vertex = has_uv_ ? 8uz : 6uz;
    float             min_x = std::numeric_limits<float>::infinity();
    float             min_y = std::numeric_limits<float>::infinity();
    float             min_z = std::numeric_limits<float>::infinity();
    float             max_x = -std::numeric_limits<float>::infinity();
    float             max_y = -std::numeric_limits<float>::infinity();
    float             max_z = -std::numeric_limits<float>::infinity();

    for (std::size_t i = 0; i < interleaved_.size(); i += floats_per_vertex)
    {
        const float x = interleaved_[i + 0];
        const float y = interleaved_[i + 1];
        const float z = interleaved_[i + 2];
        min_x           = (std::min)(min_x, x);
        min_y           = (std::min)(min_y, y);
        min_z           = (std::min)(min_z, z);
        max_x           = (std::max)(max_x, x);
        max_y           = (std::max)(max_y, y);
        max_z           = (std::max)(max_z, z);
    }

    const float cx = 0.5f * (min_x + max_x);
    const float cy = 0.5f * (min_y + max_y);
    const float cz = 0.5f * (min_z + max_z);

    for (std::size_t i = 0; i < interleaved_.size(); i += floats_per_vertex)
    {
        interleaved_[i + 0] -= cx;
        interleaved_[i + 1] -= cy;
        interleaved_[i + 2] -= cz;
    }

    reupload_();
}

void
Mesh::normalize_to_max_extent(float max_extent)
{
    if (interleaved_.empty() || max_extent <= 0.0f)
    {
        return;
    }

    const std::size_t floats_per_vertex = has_uv_ ? 8uz : 6uz;
    float             min_x = std::numeric_limits<float>::infinity();
    float             min_y = std::numeric_limits<float>::infinity();
    float             min_z = std::numeric_limits<float>::infinity();
    float             max_x = -std::numeric_limits<float>::infinity();
    float             max_y = -std::numeric_limits<float>::infinity();
    float             max_z = -std::numeric_limits<float>::infinity();

    for (std::size_t i = 0; i < interleaved_.size(); i += floats_per_vertex)
    {
        const float x = interleaved_[i + 0];
        const float y = interleaved_[i + 1];
        const float z = interleaved_[i + 2];
        min_x           = (std::min)(min_x, x);
        min_y           = (std::min)(min_y, y);
        min_z           = (std::min)(min_z, z);
        max_x           = (std::max)(max_x, x);
        max_y           = (std::max)(max_y, y);
        max_z           = (std::max)(max_z, z);
    }

    const float extent_x = max_x - min_x;
    const float extent_y = max_y - min_y;
    const float extent_z = max_z - min_z;
    const float current  = (std::max)((std::max)(extent_x, extent_y), extent_z);

    if (current <= 1e-8f)
    {
        return;
    }

    const float scale = max_extent / current;

    for (std::size_t i = 0; i < interleaved_.size(); i += floats_per_vertex)
    {
        interleaved_[i + 0] *= scale;
        interleaved_[i + 1] *= scale;
        interleaved_[i + 2] *= scale;
    }

    reupload_();
}

std::shared_ptr<Mesh>
Mesh::load(const std::filesystem::path &path)
{
    Assimp::Importer importer;

    constexpr unsigned import_flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals
        | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices
        | aiProcess_PreTransformVertices | aiProcess_FlipUVs;

    const aiScene *scene = importer.ReadFile(path.string(), import_flags);

    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0u
        || scene->mRootNode == nullptr)
    {
        throw std::runtime_error(std::format(
            "Failed to load mesh `{}`: {}", path.string(), importer.GetErrorString()));
    }

    if (scene->mNumMeshes == 0)
    {
        throw std::runtime_error(
            std::format("Mesh `{}` contains no geometry.", path.string()));
    }

    bool global_has_uv = false;
    for (unsigned m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh *amesh = scene->mMeshes[m];
        if (amesh->HasTextureCoords(0))
        {
            global_has_uv = true;
            break;
        }
    }

    std::vector<float>    interleaved;
    std::vector<unsigned> indices;

    unsigned base_vertex = 0;

    for (unsigned m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh *amesh = scene->mMeshes[m];

        for (unsigned v = 0; v < amesh->mNumVertices; ++v)
        {
            const aiVector3D &p = amesh->mVertices[v];
            interleaved.push_back(p.x);
            interleaved.push_back(p.y);
            interleaved.push_back(p.z);

            aiVector3D n{ 0.0f, 1.0f, 0.0f };
            if (amesh->HasNormals())
            {
                n = amesh->mNormals[v];
            }
            interleaved.push_back(n.x);
            interleaved.push_back(n.y);
            interleaved.push_back(n.z);

            if (global_has_uv)
            {
                if (amesh->HasTextureCoords(0))
                {
                    const aiVector3D &uv = amesh->mTextureCoords[0][v];
                    interleaved.push_back(uv.x);
                    interleaved.push_back(uv.y);
                }
                else
                {
                    interleaved.push_back(0.0f);
                    interleaved.push_back(0.0f);
                }
            }
        }

        for (unsigned f = 0; f < amesh->mNumFaces; ++f)
        {
            const aiFace &face = amesh->mFaces[f];
            if (face.mNumIndices != 3U)
            {
                throw std::runtime_error(std::format(
                    "Non-triangle face in mesh `{}` after triangulation.", path.string()));
            }

            indices.push_back(base_vertex + face.mIndices[0]);
            indices.push_back(base_vertex + face.mIndices[1]);
            indices.push_back(base_vertex + face.mIndices[2]);
        }

        base_vertex += amesh->mNumVertices;
    }

    return std::shared_ptr<Mesh>(
        new Mesh(std::move(interleaved), std::move(indices), global_has_uv));
}

}
