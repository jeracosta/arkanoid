#pragma once

#include <format>

#include "oh-my-engine/math/interval.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/hitbox_node.hpp"
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
        auto region = ome::Box::from_bounds(ome::Vec3f{ -area_[0], -3.0f, -area_[1] },
                                            ome::Vec3f{ area_[0], 0.0f, area_[1] });
        emplace_child<TerrainNode>(region).rename("Suelo");

        // Giant invisible walls, each with one face sitting exactly on an edge of the terrain,
        // so projectiles bounce off them. They enclose the forward and both side edges; the
        // backward edge is left open.
        constexpr float wall_extent = 1000.0f;

        auto forward_wall = ome::Box::from_bounds(
            ome::Vec3f{ -wall_extent, -wall_extent, -area_[1] - wall_extent },
            ome::Vec3f{ wall_extent, wall_extent, -area_[1] });
        emplace_child<ome::HitboxNode>(forward_wall.size(), forward_wall.center())
            .rename("ForwardWall");

        auto right_wall
            = ome::Box::from_bounds(ome::Vec3f{ area_[0], -wall_extent, -wall_extent },
                                    ome::Vec3f{ area_[0] + wall_extent, wall_extent, wall_extent });
        emplace_child<ome::HitboxNode>(right_wall.size(), right_wall.center()).rename("RightWall");

        auto left_wall = ome::Box::from_bounds(
            ome::Vec3f{ -area_[0] - wall_extent, -wall_extent, -wall_extent },
            ome::Vec3f{ -area_[0], wall_extent, wall_extent });
        emplace_child<ome::HitboxNode>(left_wall.size(), left_wall.center()).rename("LeftWall");

        constexpr float margin = 1.5f;
        uint            count  = config.column_count;
        for (uint side = 0; side < 2; ++side)
        {
            float side_x = (side == 0) ? -area_[0] + margin : area_[0] - margin;
            uint  base   = side * count;
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
