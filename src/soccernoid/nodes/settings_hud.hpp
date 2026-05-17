#pragma once

#include <imgui.h>

#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

// In-game overlay for inspecting/editing game state. For now it only proves the
// ImGui pipeline end to end (event capture + frame bracketing + node-emitted
// widgets); the settings controls are wired in a later step.
class SettingsHudNode : public SoccernoidNode<>
{
  public:
    void
    on_tick_() override
    {
        ImGui::Begin("Soccernoid");

        ImGui::Text("FPS: %.0f", game()->instant_frame_rate());

        ImGui::End();
    }
};

} // namespace soccernoid
