#pragma once

#include <cstdint>
#include <format>

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "soccernoid/nodes/column.hpp"
#include "soccernoid/nodes/terrain.hpp"

namespace soccernoid {

class MapNode : public ome::TransformNode
{
  public:
    struct Configuration
    {
        uint       column_count = 4;
        ome::Vec2f area         = { 12, 14 };
    };

  private:
    ome::Vec2f area_;

  public:
    const ome::Vec2f &
    area() const
    {
        return area_;
    }

    explicit MapNode(const Configuration &config)
        : area_(config.area)
    {
        auto region = ome::Box::from_bounds(
            ome::Vec3f{ -area_[0], -3.0f, -area_[1] },
            ome::Vec3f{ area_[0], 0.0f, area_[1] });
        emplace_child<TerrainNode>(region).rename("Suelo");

        constexpr float margin = 1.5f;
        uint            count  = config.column_count;
        for (uint side = 0; side < 2; ++side)
        {
            float side_x  = (side == 0) ? -area_[0] + margin : area_[0] - margin;
            uint  base    = side * count;
            for (uint i = 0; i < count; ++i)
            {
                float t = count > 1 ? static_cast<float>(i) / (count - 1) : 0.5f;
                float z = ome::lerp(-area_[1] + margin, area_[1] - margin, t);
                emplace_child<ColumnNode>()
                    .position({ side_x, 0.0f, z })
                    .rename(std::format("Column {}", base + i));
            }
        }
    }
};

} // namespace soccernoid
