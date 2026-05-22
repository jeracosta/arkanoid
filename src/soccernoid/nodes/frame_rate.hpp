#include <format>

#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

class FrameRateNode : public SoccernoidNode<>
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

} // namespace soccernoid
