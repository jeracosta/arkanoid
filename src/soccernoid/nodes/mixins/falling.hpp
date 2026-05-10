#pragma once

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "soccernoid/constants.hpp"

namespace soccernoid {

template <std::derived_from<ome::KinematicNode> TBase, float TGravityMultiplier = 1.0f>
class Falling : public TBase
{
  private:
    static constexpr ome::Vec3f force_ = ome::down * gravity_strength * TGravityMultiplier;

  public:
    void
    on_tick_() override
    {
        this->set_velocity(this->velocity() + force_ * this->game()->time.delta());

        TBase::on_tick_();
    }
};
} // namespace soccernoid
