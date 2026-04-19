#include <numbers>

#include "oh-my-engine/math/vector.hpp"

namespace ome {

static constexpr float pi = std::numbers::pi_v<float>;

namespace directions {

static constexpr Vec3f up       = { 0.0f, 1.0f, 0.0f };
static constexpr Vec3f right    = { 1.0f, 0.0f, 0.0f };
static constexpr Vec3f forward  = { 0.0f, 0.0f, 1.0f };
static constexpr Vec3f down     = -up;
static constexpr Vec3f left     = -right;
static constexpr Vec3f backward = -forward;
} // namespace directions

} // namespace ome
