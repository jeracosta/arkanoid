#include <GL/gl.h>
#include <memory>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/texture.hpp"

namespace ome {

struct Material
{
    struct BlendMode
    {
        GLenum source_factor      = GL_SRC_ALPHA;
        GLenum destination_factor = GL_ONE;
    };

    std::shared_ptr<Texture> texture    = nullptr;
    Color                    color      = Color::white();
    std::optional<BlendMode> blend_mode = std::nullopt;
};

} // namespace ome
