#pragma once

#include <GL/gl.h>

namespace ome::open_gl {

struct BlendMode
{
    GLenum source_factor      = GL_SRC_ALPHA;
    GLenum destination_factor = GL_ONE;
};

}
