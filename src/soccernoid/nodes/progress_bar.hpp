#pragma once

#include <memory>

#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/material.hpp"
#include "oh-my-engine/mesh.hpp"
#include "oh-my-engine/nodes/mesh_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace soccernoid {

class ProgressBarNode : public ome::TransformNode
{
  public:
    struct Configuration
    {
        struct
        {
            ome::Material foreground;
            ome::Material background;
            ome::Material ghosting;
        } materials;

        ome::Vec2f size;
        float      slide_duration = 0.3f;
        float      ghost_delay    = 0.0f;
    };

  private:
    Configuration                              config_;
    ome::MeshNode                             *mesh_node_ = nullptr;
    std::shared_ptr<ome::Mesh>                 mesh_;
    ome::Vec3f                                 right_;
    ome::Vec3f                                 up_;
    std::shared_ptr<ome::Interpolation<float>> progress_curve_;
    std::unique_ptr<ome::CurveProcess<float>>  progress_process_;
    float                                      current_target_        = 1.0f;
    float                                      ghost_value_           = 1.0f;
    float                                      ghost_target_          = 1.0f;
    float                                      ghost_delay_remaining_ = 0.0f;
    std::unique_ptr<ome::CurveProcess<float>>  ghost_process_;
    bool                                       ghost_active_ = false;

    bool
    has_ghost_() const
    {
        return config_.ghost_delay > 0.0f;
    }

    std::size_t
    foreground_material_index_() const
    {
        return has_ghost_() ? 2 : 1;
    }

    std::size_t
    ghost_index_() const
    {
        return 1;
    }

    void
    orient_to_camera_()
    {
        auto &camera = game()->camera;
        right_       = camera.right() * (config_.size[0] * 0.5f);
        up_          = camera.up() * (config_.size[1] * 0.5f);
    }

    void
    update_surface_(std::size_t index, float from, float to)
    {
        auto &v = mesh_->surface(index).vertices;

        v[0].position = right_ * (2.0f * from - 1.0f) - up_;
        v[1].position = right_ * (2.0f * to - 1.0f) - up_;
        v[2].position = right_ * (2.0f * to - 1.0f) + up_;
        v[3].position = right_ * (2.0f * from - 1.0f) + up_;
    }

    void
    create_mesh_()
    {
        orient_to_camera_();
        auto &camera = game()->camera;

        std::vector<ome::Mesh::Surface> surfaces;
        std::vector<ome::Material>      materials;

        auto background = ome::Mesh::Surface::billboard({ 0, 0, 0 }, config_.size, camera);
        background.material_index = surfaces.size();

        surfaces.push_back(background);
        materials.push_back(config_.materials.background);

        if (has_ghost_())
        {
            auto ghost           = ome::Mesh::Surface::billboard({ 0, 0, 0 }, config_.size, camera);
            ghost.material_index = surfaces.size();

            surfaces.push_back(ghost);
            materials.push_back(config_.materials.ghosting);
        }

        auto foreground = ome::Mesh::Surface::billboard({ 0, 0, 0 }, config_.size, camera);
        foreground.material_index = surfaces.size();

        surfaces.push_back(foreground);
        materials.push_back(config_.materials.foreground);

        auto indices
            = std::views::iota(std::size_t{ 0 }, surfaces.size()) | std::ranges::to<std::vector>();

        mesh_ = std::make_shared<ome::Mesh>(std::move(surfaces),
                                            ome::Mesh::Node{ std::move(indices) });

        mesh_node_->update_mesh([&](auto &mesh) { mesh = mesh_; });
    }

    void
    start_ghost_slide_()
    {
        auto curve = std::make_shared<ome::Interpolation<float>>(
            ghost_value_, ghost_target_, ome::EasingCurve::smoothstep());

        ghost_process_ = std::make_unique<ome::CurveProcess<float>>(curve, config_.slide_duration);
    }

  public:
    ProgressBarNode(const Configuration &config)
        : config_(config)
    {
        std::vector<ome::Material> materials;

        materials.push_back(config_.materials.background);

        if (has_ghost_())
        {
            materials.push_back(config_.materials.ghosting);
        }

        materials.push_back(config_.materials.foreground);

        mesh_node_ = &emplace_child<ome::MeshNode>(nullptr, std::move(materials));
        mesh_node_->rename("Bar");

        progress_curve_ = std::make_shared<ome::Interpolation<float>>(
            1.0f, 1.0f, ome::EasingCurve::smoothstep());

        progress_process_
            = std::make_unique<ome::CurveProcess<float>>(progress_curve_, config_.slide_duration);

        progress_process_->complete();
    }

    void
    on_mount_() override
    {
        TransformNode::on_mount_();
        create_mesh_();
    }

    bool
    is_progress_completed() const
    {
        return progress_process_->is_completed();
    }

    void
    set_progress(float target)
    {
        if (target == current_target_)
        {
            return;
        }

        current_target_ = target;

        auto old = progress_process_->value();
        if (target < old && !ghost_active_ && has_ghost_())
        {
            ghost_value_           = old;
            ghost_target_          = target;
            ghost_delay_remaining_ = config_.ghost_delay;
            ghost_process_.reset();
            ghost_active_ = true;
        }
        progress_curve_->from(old);
        progress_curve_->to(target);
        progress_process_->restart();
    }

    void
    on_tick_() override
    {
        TransformNode::on_tick_();

        orient_to_camera_();

        auto delta_time = game()->time.delta();

        progress_process_->update(delta_time);
        auto progress = progress_process_->value();

        update_surface_(foreground_material_index_(), 0.0f, progress);
        update_surface_(0, has_ghost_() ? ghost_value_ : progress, 1.0f);

        if (ghost_delay_remaining_ > 0.0f)
        {
            ghost_delay_remaining_ -= delta_time;
            update_surface_(ghost_index_(), progress, ghost_value_);

            if (ghost_delay_remaining_ <= 0.0f)
            {
                start_ghost_slide_();
            }
        }

        if (ghost_process_)
        {
            ghost_process_->update(delta_time);

            ghost_value_ = ghost_process_->value();

            update_surface_(ghost_index_(), progress, ghost_value_);
            update_surface_(0, ghost_value_, 1.0f);

            if (ghost_process_->is_completed())
            {
                ghost_active_ = false;
            }
        }

        if (has_ghost_() && !ghost_active_)
        {
            update_surface_(ghost_index_(), progress, progress);
        }
    }
};

} // namespace soccernoid
