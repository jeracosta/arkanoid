#include "application.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <print>

auto
frame(const Application::RuntimeContext &frame)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_TRIANGLES);
    {
        glColor3f(1.0, 0.0, 0.0); // Rojo
        glVertex3f(-1.5, 1.0, -6.0);
        glColor3f(0.0, 1.0, 0.0); // Verde
        glVertex3f(-2.5, -1.0, -6.0);
        glColor3f(0.0, 0.0, 1.0); // Azul
        glVertex3f(-0.5, -1.0, -6.0);
    }
    glEnd();
}

int
main()
{
    auto app = Application({
      .window = {
          .title = "Test SDL app",
          .size  = {640, 480},
      },

      .input_setup = [](auto &inputs, auto &context)
      {
          inputs.bind(SDLK_ESCAPE, KeyInput::Release, InputAction{context.stop});

          inputs.bind(SDLK_F10, KeyInput::Press, [&]{
              auto size = context.window.size();
              std::println("Toggling fullscreen mode. Current size: {} x {}", size.x, size.y);
              context.window.toggle_fullscreen();
          });
      },

      .init = []
      {
          glClearColor(0.0,0.0,0.0,1.0); // Negro
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45, (640.0 / 480.0), 0.1, 100);
      },

      .frame_logic = frame,

    });

    app.run();

    return EXIT_SUCCESS;
}
