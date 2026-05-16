#pragma once

#include <GL/gl.h>

namespace ome {

struct BlendMode
{
    GLenum source_factor      = GL_SRC_ALPHA;
    GLenum destination_factor = GL_ONE_MINUS_SRC_ALPHA;

    static constexpr BlendMode
    opaque()
    {
        return { GL_ONE, GL_ZERO };
    }

    static constexpr BlendMode
    alpha()
    {
        return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA };
    }

    static constexpr BlendMode
    additive()
    {
        return { GL_SRC_ALPHA, GL_ONE };
    }

    static constexpr BlendMode
    multiply()
    {
        return { GL_DST_COLOR, GL_ZERO };
    }

    static constexpr BlendMode
    premultiplied_alpha()
    {
        return { GL_ONE, GL_ONE_MINUS_SRC_ALPHA };
    }
};

} // namespace ome
