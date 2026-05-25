#pragma once

#include "oh-my-engine/draw_command.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"

namespace soccernoid {

class PlayerNode : public ome::KinematicNode
{
  public:
    struct Configuration
    {
        float movement_force;
        float max_speed;
        float speed_decay;

        static Configuration
        make_harry();
    };

  private:
    static std::shared_ptr<ome::Mesh>
    character_mesh_();

    static ome::Material
    character_material_();

    // Built once (the mesh is prepared a single time); on_render_ only updates its transform.
    ome::DrawCommand character_draw_{ .mesh      = character_mesh_(),
                                      .materials = { character_material_() },
                                      .transform = {} };

    static constexpr float player_radius_ = 0.2f;

    Configuration config_;

    bool aiming_;

    static constexpr float aim_sweep_speed_     = 2.0f;
    static constexpr float aim_sweep_amplitude_ = 0.7f;
    static constexpr float shoot_force_         = 5.0f;

    class AimArrowNode_ : public ome::Node
    {
      private:
        float          current_angle_ = 0.0f;
        ome::MeshNode *arrow_mesh_;

        void
        on_shoot_();

      public:
        AimArrowNode_();

        void
        on_mount_() override;

        void
        on_tick_() override;
    };

    void
    on_render_(ome::RenderFrame &frame) override;

    void
    process_movement_();

  public:
    PlayerNode(const Configuration &config);

    void
    start_aiming();

    void
    shoot(ome::Vec3f direction);

    void
    on_tick_() override;
};

} // namespace soccernoid
