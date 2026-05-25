#pragma once

#include <array>
#include <imgui.h>

#include "soccernoid/constants.hpp"
#include "soccernoid/events.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/soccernoid_node.hpp"
#include "soccernoid/settings.hpp"

namespace soccernoid {

// In-game menu: a main screen with credits, leading to settings, controls or quitting.
class SettingsHudNode : public SoccernoidNode<>
{
  private:
    enum class Screen
    {
        Main,
        Settings,
        Controls,
    };

    static constexpr float menu_font_size_   = 20.0f;
    static constexpr float panel_font_scale_ = 1.0f;

    std::array<ImFont *, static_cast<int>(TextFont::Count_)> menu_fonts_{};
    ImFont                                                  *menu_font_ = nullptr;

    bool   visible_ = false;
    Screen screen_  = Screen::Main;

    void
    toggle_()
    {
        visible_ = !visible_;
        screen_  = Screen::Main;
        game()->window.set_relative_mouse_mode(!visible_);
        game()->settings.set(settings::time::Paused{ visible_ });
    }

    void
    begin_centered_panel_(const char *title, bool *open = nullptr)
    {
        const auto *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2{ 0.5f, 0.5f });
        ImGui::SetNextWindowSize(ImVec2{ viewport->Size.x * 0.8f, viewport->Size.y * 0.8f },
                                 ImGuiCond_Always);

        constexpr auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                               | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin(title, open, flags);
        ImGui::SetWindowFontScale(panel_font_scale_);
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
            auto  rgb  = color.rgb_f();
            float c[3] = { rgb[0], rgb[1], rgb[2] };
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

        const char *views[] = { "Third person", "First person", "Freecam" };

        int current = static_cast<int>(settings.get<settings::camera::View>().value);
        if (ImGui::Combo("View", &current, views, IM_ARRAYSIZE(views)))
        {
            settings.set(settings::camera::View{ static_cast<CameraView>(current) });
        }
    }

    static constexpr int
    font_index_(TextFont font)
    {
        return static_cast<int>(font);
    }

    void
    update_menu_font_(const settings::ui::TextFont &font)
    {
        menu_font_ = menu_fonts_[font_index_(font.value)];
    }

    void
    font_combo_()
    {
        auto &settings = game()->settings;

        const char *fonts[] = { "Arial", "Comic Sans", "DejaVu Sans" };

        int current = font_index_(settings.get<settings::ui::TextFont>().value);
        if (ImGui::Combo("Text font", &current, fonts, IM_ARRAYSIZE(fonts)))
        {
            settings.set(settings::ui::TextFont{ static_cast<TextFont>(current) });
        }
    }

    void
    skybox_combo_()
    {
        auto &settings = game()->settings;

        const auto &skyboxes = textures.skybox;
        if (skyboxes.empty())
        {
            return;
        }

        int current = settings.get<settings::render::Skybox>().value;
        if (current < 0 || current >= static_cast<int>(skyboxes.size()))
        {
            current = 0;
        }

        // Names are loaded at runtime, so populate the combo manually.
        if (ImGui::BeginCombo("Skybox", skyboxes[current].name.c_str()))
        {
            for (int i = 0; i < static_cast<int>(skyboxes.size()); ++i)
            {
                bool selected = i == current;
                if (ImGui::Selectable(skyboxes[i].name.c_str(), selected))
                {
                    settings.set(settings::render::Skybox{ i });
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void
    main_screen_()
    {
        begin_centered_panel_("Soccernoid");

        ImGui::SetWindowFontScale(panel_font_scale_ * 1.4f);
        ImGui::TextUnformatted("Soccernoid");
        ImGui::SetWindowFontScale(panel_font_scale_);

        ImGui::TextDisabled("Introducción a la Computación Gráfica - FING, UdelaR - 2026");
        ImGui::Spacing();
        ImGui::TextDisabled("Desarrollado por Jerónimo Acosta, Florencia Artucio, Joaquín Sande");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const ImVec2 button_size{ ImGui::GetContentRegionAvail().x, 0.0f };

        if (ImGui::Button("Configuración", button_size))
        {
            screen_ = Screen::Settings;
        }
        if (ImGui::Button("Controles", button_size))
        {
            screen_ = Screen::Controls;
        }
        if (ImGui::Button("Salir", button_size))
        {
            game()->events.emit(AppTerminated{});
        }

        ImGui::End();
    }

    void
    settings_screen_()
    {
        ImGui::SetNextWindowSize(ImVec2{ 760.0f, 0.0f }, ImGuiCond_FirstUseEver);

        bool open = true;
        ImGui::Begin("Configuración", &open, ImGuiWindowFlags_NoSavedSettings);

        ImGui::Text("FPS: %.0f", game()->instant_frame_rate());

        ImGui::SeparatorText("Time");
        checkbox_<settings::time::Paused>("Paused");
        slider_<settings::time::Speed>("Speed", 0.0f, 4.0f, "%.2f");

        ImGui::SeparatorText("Camera");
        view_combo_();
        slider_<settings::camera::MouseSensitivity>("Mouse sensitivity", 0.001f, 0.1f);
        slider_<settings::camera::MovementSpeed>("Movement speed", 0.0f, 20.0f, "%.1f");

        ImGui::SeparatorText("Render");
        font_combo_();
        skybox_combo_();
        checkbox_<settings::render::ShowWireframes>("Wireframe");
        checkbox_<settings::render::ShowTextures>("Textures");
        checkbox_<settings::render::SmoothShading>("Smooth shading");
        checkbox_<settings::render::ShowHitboxes>("Show hitboxes");

        if (ImGui::CollapsingHeader("Global light"))
        {
            global_light_controls_();
        }

        ImGui::SeparatorText("Window");
        checkbox_<settings::window::Fullscreen>("Fullscreen");
        checkbox_<settings::render::ShowFrameRate>("Show frame rate");

        ImGui::End();

        if (!open)
        {
            toggle_();
        }
    }

    void
    controls_screen_()
    {
        struct Control
        {
            const char *keys;
            const char *action;
        };

        static constexpr Control controls[] = {
            { "Flechas Izq / Der", "Mover al jugador" },
            { "Flechas Arr / Ab", "Levitar más alto / más bajo" },
            { "Enter", "Apuntar y disparar" },
            { "Mouse", "Rotar la cámara" },
            { "Rueda del mouse", "Acercar / alejar (FOV)" },
            { "W A S D", "Mover la cámara (primera persona)" },
            { "Espacio / Ctrl", "Subir / bajar la cámara" },
            { "Shift", "Acelerar la cámara" },
            { "V", "Cambiar vista" },
            { "P", "Pausar / reanudar" },
            { "+ / -", "Velocidad del juego" },
            { "F11", "Pantalla completa" },
            { "Esc", "Abrir / cerrar el menú" },
            { "Q", "Salir del juego" },
        };

        bool open = true;
        begin_centered_panel_("Controles", &open);

        ImGui::SeparatorText("Controles");

        constexpr auto table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
                                     | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("controls", 2, table_flags))
        {
            ImGui::TableSetupColumn("Tecla", ImGuiTableColumnFlags_WidthFixed, 540.0f);
            ImGui::TableSetupColumn("Acción");
            ImGui::TableHeadersRow();

            for (const auto &control : controls)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(control.keys);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(control.action);
            }

            ImGui::EndTable();
        }

        ImGui::End();

        if (!open)
        {
            toggle_();
        }
    }

  public:
    void
    on_mount_() override
    {
        auto *fonts = ImGui::GetIO().Fonts;
        fonts->AddFontDefault();

        const auto font_path = FilesystemPaths::assets / "fonts";

        menu_fonts_[font_index_(TextFont::Arial)] = fonts->AddFontFromFileTTF(
            (font_path / "NimbusSans-Regular.otf").string().c_str(), menu_font_size_);
        menu_fonts_[font_index_(TextFont::ComicSans)] = fonts->AddFontFromFileTTF(
            (font_path / "comici.ttf").string().c_str(), menu_font_size_);
        menu_fonts_[font_index_(TextFont::DejaVuSans)] = fonts->AddFontFromFileTTF(
            (font_path / "DejaVuSans.ttf").string().c_str(), menu_font_size_);

        update_menu_font_(game()->settings.get<settings::ui::TextFont>());
        hold(game()->settings.bind(&SettingsHudNode::update_menu_font_, this));

        hold(game()->input.bind(Action::ToggleHud, [this] { toggle_(); }));
    }

    void
    on_tick_() override
    {
        if (!visible_)
        {
            return;
        }

        if (menu_font_)
        {
            ImGui::PushFont(menu_font_);
        }

        switch (screen_)
        {
        case Screen::Main:
            main_screen_();
            break;
        case Screen::Settings:
            settings_screen_();
            break;
        case Screen::Controls:
            controls_screen_();
            break;
        }

        if (menu_font_)
        {
            ImGui::PopFont();
        }
    }
};

} // namespace soccernoid
