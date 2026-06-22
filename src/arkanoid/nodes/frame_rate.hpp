#include <format>

#include "arkanoid/nodes/arkanoid_node.hpp"
#include "arkanoid/settings.hpp"

namespace arkanoid {

class FrameRateNode : public ArkanoidNode<>
{
  public:
    void
    on_tick_() override
    {
        if (!game()->settings.get<settings::render::ShowFrameRate>())
        {
            return;
        }

        log(std::format("FPS: {}", game()->instant_frame_rate()));
    }
};

} // namespace arkanoid
