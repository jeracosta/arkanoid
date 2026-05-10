#pragma once

#include "oh-my-engine/color.hpp"

namespace soccernoid {

static constexpr float gravity_strength = 9.81f;

static constexpr float despawn_distance = 60.0f;

// Fog
static constexpr float fog_start = 25.0f;
static constexpr float fog_end   = 50.0f;

// Terrain box depth below the field plane (at y=0)
static constexpr float terrain_box_depth = 40.0f;

struct Palette
{
    ome::Color fog;
    ome::Color ball;
    ome::Color grass;
    ome::Color dirt;
    ome::Color goal;
    ome::Color red_kit;
    ome::Color skin;
};

static const inline Palette palette = {
    .fog     = ome::Color::rgb(0, 0, 0),
    .ball    = ome::Color::rgb(210, 50, 255),
    .grass   = ome::Color::rgb(60, 175, 45),
    .dirt    = ome::Color::rgb(105, 65, 35),
    .goal    = ome::Color::rgb(230, 242, 217),
    .red_kit = ome::Color::rgb(200, 30, 30),
    .skin    = ome::Color::rgb(230, 190, 150),
};

} // namespace soccernoid
