#pragma once

#include <numbers>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

static constexpr float pi = std::numbers::pi_v<float>;

static constexpr Vec3f up       = { 0.0f, 1.0f, 0.0f };
static constexpr Vec3f right    = { 1.0f, 0.0f, 0.0f };
static constexpr Vec3f forward  = { 0.0f, 0.0f, -1.0f };
static constexpr Vec3f down     = -up;
static constexpr Vec3f left     = -right;
static constexpr Vec3f backward = -forward;

namespace ecs {

#ifndef OME_ECS_MAX_ENTITIES
#define OME_ECS_MAX_ENTITIES (1 << 16)
#endif

#ifndef OME_ECS_MAX_COMPONENT_TYPES
#define OME_ECS_MAX_COMPONENT_TYPES (1 << 8)
#endif

static constexpr std::size_t max_entities        = OME_ECS_MAX_ENTITIES;
static constexpr std::size_t max_component_types = OME_ECS_MAX_COMPONENT_TYPES;

} // namespace ecs

} // namespace ome
