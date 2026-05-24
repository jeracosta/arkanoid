#pragma once

#include <imgui.h>

#include "soccernoid/input.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// In-game overlay for inspecting and editing game settings
class SettingsHudNode : public SoccernoidNode<>
{
  private:
    bool visible_ = false;

    void
    toggle_()
    {
        visible_ = !visible_;
        game()->window.set_relative_mouse_mode(!visible_);
        game()->settings.set(settings::time::Paused{ visible_ });
    }

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
    global_light_controls_()
    {
        auto &settings = game()->settings;
        auto  light    = settings.get<settings::GlobalLight>();

        bool changed = false;

        changed |= ImGui::DragFloat3("Direction", light.direction.data(), 0.01f, -1.0f, 1.0f);

        auto color_edit = [&](const char *label, ome::Color &color)
        {
            auto  rgb    = color.rgb_f();
            float c[3]   = { rgb[0], rgb[1], rgb[2] };
            if (ImGui::ColorEdit3(label, c))
            {
                color   = ome::Color::rgb(c[0], c[1], c[2]);
                changed = true;
            }
        };

        color_edit("Ambient", light.ambient);
        color_edit("Diffuse", light.diffuse);
        color_edit("Specular", light.specular);

        if (changed)
        {
            settings.set(light);
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
    on_mount_() override
    {
        hold(game()->input.bind(Action::ToggleHud, [this] { toggle_(); }));
    }

    void
    on_tick_() override
    {
        if (!visible_)
        {
            return;
        }

        ImGui::Begin("Soccernoid");

        ImGui::Text("FPS: %.0f", game()->instant_frame_rate());

        ImGui::SeparatorText("Time");
        checkbox_<settings::time::Paused>("Paused");
        slider_<settings::time::Speed>("Speed", 0.0f, 4.0f, "%.2f");

        ImGui::SeparatorText("Camera");
        view_combo_();
        slider_<settings::camera::MouseSensitivity>("Mouse sensitivity", 0.001f, 0.1f);
        slider_<settings::camera::MovementSpeed>("Movement speed", 0.0f, 20.0f, "%.1f");

        ImGui::SeparatorText("Render");
        checkbox_<settings::render::ShowWireframes>("Wireframe");

        if (ImGui::CollapsingHeader("Global light"))
        {
            global_light_controls_();
        }

        ImGui::SeparatorText("Window");
        checkbox_<settings::window::Fullscreen>("Fullscreen");
        checkbox_<settings::render::ShowFrameRate>("Show frame rate");

        ImGui::End();
    }
};

} // namespace soccernoid
