#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/input.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/input.hpp"

namespace soccernoid {

enum class CameraView
{
    FirstPerson,
    ThirdPerson,
    Count_,
};

inline CameraView
succesor(CameraView view)
{
    return static_cast<CameraView>((static_cast<int>(view) + 1)
                                   % static_cast<int>(CameraView::Count_));
}

class CameraControlNode : public ome::Node
{
  private:
    ome::Camera *camera_;
    CameraView   view_ = CameraView::ThirdPerson;

    // TODO: Make these configurable
    static constexpr float sensitivity_ = 0.01f;
    static constexpr float base_speed_  = 1.0;

    template <CameraView>
    void
    relocate_camera_();

    void
    on_mouse_motion_(const ome::input::MouseMotionInput &input)
    {

        auto [yaw, pitch] = -input.delta * sensitivity_;

        camera_->rotate(yaw, camera_->up());
        camera_->rotate(pitch, camera_->right());
    }

    template <CameraView TView>
    inline void
    set_view_()
    {
        view_ = TView;

        relocate_camera_<TView>();
    }

    void
    process_movement_()
    {
        if (view_ != CameraView::FirstPerson)
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

        auto direction = camera_->orientation() * normal(raw_direction);

        auto is_sprinting = game()->input.is_pressed(Action::CameraSprint);

        auto speed_factor = is_sprinting ? 2.0f : 1.0f;
        auto speed        = base_speed_ * speed_factor;

        auto velocity     = direction * speed;
        auto displacement = velocity * game()->time.unscaled.delta();

        camera_->move_target(displacement);
    }

  public:
    void
    on_mount_() override
    {
        camera_ = &game()->camera;

        hold(game()->input.bind(Action::ChangeView, [this] { set_view(succesor(view_)); }));

        auto mouse_motion_handler = &CameraControlNode::on_mouse_motion_;
        hold(game()->input.bind(mouse_motion_handler, this));

        set_view(view_);
    }

    CameraView
    current_view() const
    {
        return view_;
    }

    void
    set_view(CameraView view);

    void
    on_tick_() override
    {
        process_movement_();
    }
};

inline void
CameraControlNode::set_view(CameraView view)
{
    switch (view)
    {
    case CameraView::FirstPerson:
        set_view_<CameraView::FirstPerson>();
        break;
    case CameraView::ThirdPerson:
        set_view_<CameraView::ThirdPerson>();
        break;
    default:
        throw std::runtime_error("Unsuported camera view");
    }
}

template <>
inline void
CameraControlNode::relocate_camera_<CameraView::FirstPerson>()
{
    auto distance      = 0.1f;
    auto delta_distace = camera_->distance() - distance;
    auto delta_target  = camera_->backward() * delta_distace;

    camera_->move_target(delta_target);
    camera_->distance(distance);
}

template <>
inline void
CameraControlNode::relocate_camera_<CameraView::ThirdPerson>()
{
    camera_->target({ 0, 0, 0 });
    camera_->distance(5);
    camera_->orientate({});
}

} // namespace soccernoid
