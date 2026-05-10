#pragma once

#include "oh-my-engine/color.hpp"

namespace soccernoid {

static constexpr float gravity_strength = 9.81f;

static constexpr float despawn_distance = 60.0f;

// Fog
static constexpr float fog_start = 25.0f;
static constexpr float fog_end   = 50.0f;

static const ome::Color fog_color = ome::Color::rgb(0.0f, 0.0f, 0.0f);

// Terrain box depth below the field plane (at y=0)
static constexpr float terrain_box_depth = 40.0f;

} // namespace soccernoid
