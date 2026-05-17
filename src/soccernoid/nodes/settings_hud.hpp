#pragma once

#include <imgui.h>

#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// In-game overlay for inspecting and editing game state. It is a pure view over
// Settings: it reads values with get<T>() and writes them back with set<T>().
// Because set<T>() only emits on change and the control nodes already subscribe
// to those settings, editing a widget drives the game with no extra wiring.
class SettingsHudNode : public SoccernoidNode<>
{
  private:
    template <class T>
    void
    checkbox_(const char *label)
    {
        auto &settings = game()->settings;

        bool value = settings.get<T>().value;
        if (ImGui::Checkbox(label, &value))
        {
            settings.set(T{ value });
        }
    }

    template <class T>
    void
    slider_(const char *label, float min, float max, const char *format = "%.3f")
    {
        auto &settings = game()->settings;

        float value = settings.get<T>().value;
        if (ImGui::SliderFloat(label, &value, min, max, format))
        {
            settings.set(T{ value });
        }
    }

    void
    view_combo_()
    {
        auto &settings = game()->settings;

        const char *views[] = { "First person", "Third person" };

        int current = static_cast<int>(settings.get<settings::camera::View>().value);
        if (ImGui::Combo("View", &current, views, IM_ARRAYSIZE(views)))
        {
            settings.set(settings::camera::View{ static_cast<CameraView>(current) });
        }
    }

  public:
    void
    on_tick_() override
    {
        ImGui::Begin("Soccernoid");

        ImGui::Text("FPS: %.0f", game()->instant_frame_rate());

        ImGui::SeparatorText("Time");
        checkbox_<settings::time::Paused>("Paused");
        slider_<settings::time::Speed>("Speed", 0.0f, 4.0f, "%.2f");

        ImGui::SeparatorText("Camera");
        view_combo_();
        slider_<settings::camera::MouseSensitivity>("Mouse sensitivity", 0.001f, 0.1f);
        slider_<settings::camera::MovementSpeed>("Movement speed", 0.0f, 20.0f, "%.1f");

        ImGui::SeparatorText("Window");
        checkbox_<settings::window::Fullscreen>("Fullscreen");
        checkbox_<settings::render::ShowFrameRate>("Show frame rate");

        ImGui::End();
    }
};

} // namespace soccernoid
