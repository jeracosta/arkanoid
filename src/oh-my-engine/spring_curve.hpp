#pragma once

#include <cmath>
#include <memory>

#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/math/vector.hpp"

namespace ome {

struct SpringCurve : Curve<float>
{
    std::shared_ptr<Curve<float>> envelope_;
    float                         amplitude_ = 0.25f;
    float                         freq_      = 6.0f;

    float
    operator()(float t) const override
    {
        float e = (*envelope_)(t);
        return 1.0f + amplitude_ * e * std::sin(e * freq_ * std::numbers::pi_v<float>);
    }
};

} // namespace ome
