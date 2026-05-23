#pragma once

#include <cmath>

#include "oh-my-engine/curve.hpp"

namespace ome {

struct HarmonicOscillatorCurve : Curve<float>
{
    float initial_amplitude   = 0.2f;
    float decay_strength      = 4.0f;
    float angular_frequency   = 20.0f * std::numbers::pi_v<float>;
    float phase_offset        = 0.0f;

    float
    operator()(float t) const override
    {
        return 1.0f + initial_amplitude * std::exp(-decay_strength * t)
                          * std::cos(angular_frequency * t + phase_offset);
    }
};

} // namespace ome
