#pragma once

#include <imgui.h>

#include "soccernoid/nodes/soccernoid_node.hpp"

namespace soccernoid {

class VictoryScreenNode : public SoccernoidNode<>
{
  public:
    void
    on_tick_() override
    {
        const auto *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->GetCenter(),
                                ImGuiCond_Always,
                                ImVec2{ 0.5f, 0.5f });

        constexpr auto flags = ImGuiWindowFlags_NoDecoration
                               | ImGuiWindowFlags_AlwaysAutoResize
                               | ImGuiWindowFlags_NoMove
                               | ImGuiWindowFlags_NoSavedSettings
                               | ImGuiWindowFlags_NoFocusOnAppearing
                               | ImGuiWindowFlags_NoInputs;

        ImGui::Begin("##victory-screen", nullptr, flags);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.15f, 1.0f, 0.3f, 1.0f });
        ImGui::SetWindowFontScale(4.0f);
        ImGui::Text("VICTORY!");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImGui::End();
    }
};

} // namespace soccernoid
