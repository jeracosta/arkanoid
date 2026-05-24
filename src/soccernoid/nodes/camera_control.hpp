#include <optional>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/input.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/player.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

struct CameraShot
{
    ome::Vec3f       target;
    float            distance;
    ome::Orientation orientation;

    CameraShot
    operator*(float s) const
    {
        return { target * s, distance * s, orientation * s };
    }

    CameraShot
    operator+(const CameraShot &other) const
    {
        return { target + other.target,
                 distance + other.distance,
                 orientation + other.orientation };
    }
};

using CameraTransition = ome::CurveProcess<CameraShot>;

class CameraControlNode : public SoccernoidNode<>
{
  public:
    struct Settings
    {
        float sprint_multiplier   = 2.0f;
        float transition_duration = 0.5f;
    };

  private:
    using MouseSensitivity = settings::camera::MouseSensitivity;
    using MovementSpeed    = settings::camera::MovementSpeed;
    using View             = settings::camera::View;

    static constexpr float first_person_eye_height_ = 1.4f;
    static constexpr float first_person_lookahead_  = 0.8f;
    static constexpr float first_person_distance_   = 2.4f;

    ome::Camera *camera_;
    Settings     settings_;
    CameraView   view_ = CameraView::ThirdPerson;

    std::optional<CameraTransition> transition_;

    bool
    is_transitioning_() const
    {
        return transition_.has_value();
    }

    void
    on_mouse_motion_(const ome::input::MouseMotionInput &input)
    {
        if (is_transitioning_() || !game()->window.is_relative_mouse_mode())
        {
            return;
        }

        // FIXME: Clamp pitch to avoid turning over

        auto mouse_sensitivity        = game()->settings.get<MouseSensitivity>().value;
        auto [delta_yaw, delta_pitch] = -input.delta * mouse_sensitivity;

        auto yaw_axis = view_ == CameraView::ThirdPerson ? camera_->up() : ome::up;
        camera_->rotate(delta_yaw, yaw_axis);

        camera_->rotate(delta_pitch, camera_->right());
    }

    void
    on_mouse_wheel_(const ome::input::MouseWheelInput &input)
    {
        if (is_transitioning_() || !game()->window.is_relative_mouse_mode())
        {
            return;
        }

        auto current_fov = camera_->fov_degrees();
        camera_->fov_degrees(current_fov - input.delta[1]);
    }

    CameraShot
    current_shot_() const
    {
        return { camera_->target(), camera_->distance(), camera_->orientation() };
    }

    CameraShot
    first_person_shot_(ome::Orientation orientation)
    {
        auto *player = ome::find_descendant<PlayerNode>(game()->root_node());

        if (!player)
        {
            return current_shot_();
        }

        auto eye
            = player->transform<ome::Space::World>().position + ome::up * first_person_eye_height_;

        return { eye + orientation.forward() * first_person_lookahead_,
                 first_person_distance_,
                 orientation };
    }

    void
    follow_player_()
    {
        auto shot = first_person_shot_(camera_->orientation());

        camera_->target(shot.target);
        camera_->distance(shot.distance);
    }

    CameraShot
    shot_for_view_(CameraView view)
    {
        switch (view)
        {
        case CameraView::FirstPerson:
        {
            return first_person_shot_({});
        }
        case CameraView::Freecam:
        {
            return { { 0.0f, 1.7f, 0.0f }, 0.1f, {} };
        }
        case CameraView::ThirdPerson:
        {
            ome::Orientation orientation;
            orientation.steer_pitch(-1.0f);

            return { { 0, 0, 0 }, 25.0f, orientation };
        }
        default:
            throw std::runtime_error("Unsupported camera view");
        }
    }

    void
    apply_view_(CameraView view)
    {
        if (view == view_)
        {
            return;
        }

        view_ = view;
        start_transition_();
    }

    void
    snap_to_view_(CameraView view)
    {
        view_ = view;

        auto shot = shot_for_view_(view);
        camera_->target(shot.target);
        camera_->distance(shot.distance);
        camera_->orientate(shot.orientation);
    }

    void
    start_transition_()
    {
        CameraShot from{ camera_->target(), camera_->distance(), camera_->orientation() };
        CameraShot to = shot_for_view_(view_);

        auto curve = std::make_shared<ome::Interpolation<CameraShot>>(
            from, to, ome::EasingCurve::smoothstep());
        transition_.emplace(curve, 1.0f / settings_.transition_duration);
    }

    void
    process_movement_()
    {
        if (is_transitioning_() || view_ != CameraView::Freecam)
        {
            return;
        }

        struct MoveSpecification
        {
            Action     action;
            ome::Vec3f direction;

            operator const ome::Vec3f &() const
            {
                return direction;
            }
        };

        // clang-format off

        // FIXME: Moving up and down should not be relative to camera orientation

        static auto moves = std::to_array<MoveSpecification>({
            { Action::CameraForward,  ome::forward },
            { Action::CameraBackward, ome::backward },
            { Action::CameraLeft,     ome::left },
            { Action::CameraRight,    ome::right },
            { Action::CameraUp,       ome::up },
            { Action::CameraDown,     ome::down },
        });

        auto is_active = [this](const MoveSpecification &move)
        {
            return game()->input.is_pressed(move.action);
        };

        // clang-format on

        auto active_moves = moves | std::views::filter(is_active);

        auto raw_direction = std::ranges::fold_left(active_moves, ome::Vec3f{}, std::plus{});

        if (norm(raw_direction) == 0)
        {
            return;
        }

        auto direction = camera_->orientation() * normalized(raw_direction);

        auto is_sprinting = game()->input.is_pressed(Action::CameraSprint);

        auto speed_factor = is_sprinting ? settings_.sprint_multiplier : 1.0f;
        auto speed        = game()->settings.get<MovementSpeed>().value * speed_factor;

        auto velocity     = direction * speed;
        auto displacement = velocity * game()->time.unscaled.delta();

        camera_->move_target(displacement);
    }

  public:
    CameraControlNode(const Settings &settings)
        : settings_(settings)
    {
    }

    void
    on_mount_() override
    {
        camera_ = &game()->camera;

        hold(game()->input.bind(Action::ChangeView, [this] {
            game()->settings.set<View>(succesor(game()->settings.get<View>().value));
        }));

        auto mouse_motion_handler = &CameraControlNode::on_mouse_motion_;
        hold(game()->input.bind(mouse_motion_handler, this));

        auto mouse_wheel_handler = &CameraControlNode::on_mouse_wheel_;
        hold(game()->input.bind(mouse_wheel_handler, this));

        hold(game()->settings.bind([this](const View &view) { apply_view_(view); }));

        snap_to_view_(game()->settings.get<View>());
    }

    CameraView
    current_view() const
    {
        return view_;
    }

    void
    set_view(CameraView view)
    {
        game()->settings.set<View>(view);
    }

    void
    on_tick_() override
    {
        process_movement_();

        if (transition_)
        {
            transition_->update(game()->time.unscaled.delta());

            auto shot        = transition_->value();
            shot.orientation = ome::Orientation(glm::normalize(glm::quat(shot.orientation)));

            camera_->target(shot.target);
            camera_->distance(shot.distance);
            camera_->orientate(shot.orientation);

            if (transition_->is_completed())
            {
                transition_.reset();
            }
        }

        if (!transition_ && view_ == CameraView::FirstPerson)
        {
            follow_player_();
        }
    }
};

} // namespace soccernoid
