#include "oh-my-engine/debug_ui.hpp"

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_sdl2.h>

#include "oh-my-engine/window.hpp"

namespace ome {

DebugUi::DebugUi(Window &window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::GetIO().IniFilename = nullptr; // don't persist layout to disk

    ImGui::StyleColorsDark();

    // DebugUi is a friend of Window, so it may read the raw SDL handles.
    ImGui_ImplSDL2_InitForOpenGL(window.window_, window.gl_context_);
    ImGui_ImplOpenGL2_Init();
}

DebugUi::~DebugUi()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void
DebugUi::begin_frame()
{
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void
DebugUi::end_frame()
{
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

std::optional<SDL_Event>
DebugUi::handle(const SDL_Event &event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);

    const auto &io = ImGui::GetIO();

    switch (event.type)
    {
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
        if (io.WantCaptureMouse)
        {
            return std::nullopt;
        }
        break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_TEXTINPUT:
    case SDL_TEXTEDITING:
        if (io.WantCaptureKeyboard)
        {
            return std::nullopt;
        }
        break;

    default:
        break;
    }

    return event;
}

} // namespace ome
