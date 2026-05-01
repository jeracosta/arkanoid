#include <format>

#include "oh-my-engine/node.hpp"

namespace soccernoid {

class FrameRateNode : public ome::Node
{
  public:
    void
    on_tick_() override
    {
        log(std::format("FPS: {}", game()->instant_frame_rate()));
    }
};

} // namespace soccernoid
