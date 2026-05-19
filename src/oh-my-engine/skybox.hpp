#include <memory>

#include "oh-my-engine/texture.hpp"

namespace ome {

struct Skybox
{
    float size;

    struct Textures
    {
        std::shared_ptr<Texture> front;
        std::shared_ptr<Texture> back;
        std::shared_ptr<Texture> left;
        std::shared_ptr<Texture> right;
        std::shared_ptr<Texture> top;
        std::shared_ptr<Texture> bottom;
    } textures;
};

} // namespace ome

namespace ome::open_gl {

void
render(const Skybox &skybox);

} // namespace ome::open_gl
