#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/node.hpp"

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

    template <CameraView>
    void
    set_view_();

  public:
    void
    on_mount_() override
    {
        camera_ = &game()->camera;
    }

    CameraView
    current_view() const
    {
        return view_;
    }

    void
    set_view(CameraView view);
};

template <>
inline void
CameraControlNode::set_view_<CameraView::FirstPerson>()
{
    view_ = CameraView::FirstPerson;

    auto distance      = 0.1f;
    auto delta_distace = camera_->distance() - distance;
    auto delta_target  = camera_->backward() * delta_distace;

    camera_->move_target(delta_target);
    camera_->distance(distance);
}

template <>
inline void
CameraControlNode::set_view_<CameraView::ThirdPerson>()
{
    view_ = CameraView::ThirdPerson;

    camera_->distance(5);
    camera_->orientate({});
}

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

} // namespace soccernoid
