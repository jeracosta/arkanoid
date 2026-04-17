#include "oh-my-engine/game.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <print>

glm::vec3 color;

int
main()
{
    ome::game::run({
      .window = {
            .title = "Test SDL app",
            .size  = {640, 480},
      },

      .configure_input = [](auto &inputs, auto &game)
      {
          using enum ome::input::KeyInput;

          inputs.keyboard.bind(SDLK_ESCAPE, Release, [&]{ game.stop(); });

          inputs.keyboard.bind(SDLK_F10, Press, [&]{
              auto size = game.window.size();
              std::println("Toggling fullscreen mode. Current size: {} x {}", size.x, size.y);
              game.window.toggle_fullscreen();
          });

          inputs.mouse_motion.bind([&](auto mouse){
              color.x = static_cast<float>(mouse.position.x) / game.window.size().x;
              color.y = static_cast<float>(mouse.position.y) / game.window.size().y;
          });
      },

      .on_init = []
      {
          glClearColor(0.0,0.0,0.0,1.0); // Negro
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45, (640.0 / 480.0), 0.1, 100);
      },

      .on_update = [](auto &)
      {
          glClear(GL_COLOR_BUFFER_BIT);
          glClear(GL_DEPTH_BUFFER_BIT);
      
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
      
          glBegin(GL_TRIANGLES);
          {
              glColor3f(color.x, color.y, color.z);
              glVertex3f(-1.5, 1.0, -6.0);
              glVertex3f(-2.5, -1.0, -6.0);
              glVertex3f(-0.5, -1.0, -6.0);
          }
          glEnd();
      }
    });

    return EXIT_SUCCESS;
}
