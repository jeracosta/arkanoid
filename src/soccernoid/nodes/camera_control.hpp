#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/input.hpp"
#include "oh-my-engine/node.hpp"
#include "soccernoid/actions.hpp"

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
    static constexpr float base_speed_  = 0.02f;

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

#define CHECK_AND_MOVE(action, direction)                                                          \
    if (game()->input.is_pressed(action))                                                          \
    {                                                                                              \
        camera_->move_target(camera_->direction() * speed);                                        \
    }

    void
    tick_() override
    {
        if (view_ == CameraView::FirstPerson)
        {
            auto speed_factor = game()->input.is_pressed(Action::CameraSprint) ? 2.0f : 1.0f;
            auto speed        = base_speed_ * speed_factor;

            CHECK_AND_MOVE(Action::CameraForward, forward);
            CHECK_AND_MOVE(Action::CameraBackward, backward);
            CHECK_AND_MOVE(Action::CameraLeft, left);
            CHECK_AND_MOVE(Action::CameraRight, right);
            CHECK_AND_MOVE(Action::CameraUp, up);
            CHECK_AND_MOVE(Action::CameraDown, down);
        }
    }

#undef CHECK_AND_MOVE
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
