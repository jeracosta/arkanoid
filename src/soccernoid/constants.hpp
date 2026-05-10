#pragma once

#include "oh-my-engine/color.hpp"

namespace soccernoid {

static constexpr float gravity_strength = 9.81f;

static constexpr float despawn_distance = 60.0f;

static constexpr struct
{
    float start = 25.0f;
    float end   = 50.0f;
} fog;

static constexpr float terrain_box_depth = fog.end;

static constexpr struct
{
    float mouse_sensitivity   = 0.01f;
    float movement_speed      = 5.0f;
    float sprint_multiplier   = 2.0f;
    float transition_duration = 0.25f;
} camera;

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
