#pragma once

#include "oh-my-engine/math/curve.hpp"

namespace ome {

// Simple wrapper for a curve evaluation with an internal progress state.
// Represents interpolation between two values using normalized time [0, 1].
template <class T>
struct Interpolation
{
    ome::math::ParametricCurve<T> curve    = ome::math::ParametricCurve<T>::line({ 0, 1 });
    float                         progress = 0.0f;

    void
    from(const T &from)
    {
        if (!curve.rising())
        {
            curve.domain.min = from;
        }
        else
        {
            curve.domain.max = from;
        }
    }

    void
    to(const T &to)
    {
        if (!curve.rising())
        {
            curve.domain.max = to;
        }
        else
        {
            curve.domain.min = to;
        }
    }

    void
    flip()
    {
        curve.flip_domain();
        progress = 1.0f - progress;
    }

    void
    update(float delta)
    {
        progress += delta;
        progress = std::clamp(progress, 0.0f, 1.0f);
    }

    bool
    is_completed()
    {
        return progress >= 1.0f;
    }

    void
    complete()
    {
        progress = 1.0f;
    }

    void
    restart()
    {
        progress = 0.0f;
    }

    T
    value() const
    {
        return curve(progress);
    }
};

} // namespace ome
