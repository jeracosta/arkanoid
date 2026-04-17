#include "oh-my-engine/game.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <print>
#include <unistd.h>

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

          inputs.keyboard.bind(SDLK_SPACE,  Release, [&]{ 
            game.time.toggle_pause(); 
            std::puts(game.time.is_paused() ? "Paused" : "Resumed");
          });

          inputs.keyboard.bind(SDLK_UP, Release, [&]{
            game.time.speed(game.time.speed() * 1.5);
            std::println("Speed: {:.2f} steps/s", game.time.speed());
          });

          inputs.keyboard.bind(SDLK_DOWN, Release, [&]{
            game.time.speed(game.time.speed() / 1.5);
            std::println("Speed: {:.2f} steps/s", game.time.speed());
          });

          inputs.keyboard.bind(SDLK_r, Release, [&]{
            game.time.speed(-game.time.speed());
            std::println("Time direction: {}", game.time.speed() > 0 ? "forward" : "backward");
          });

      },

      .on_init = []
      {
          glClearColor(0.0,0.0,0.0,1.0); // Negro
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45, (640.0 / 480.0), 0.1, 100);
      },

      .on_update = [](auto &game)
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
      
          glTranslatef(-1.5f, 0.0f, -5 - sin(game.time.elapsed()) / 0.1);
      
          glBegin(GL_QUADS);
          {
              glColor3f(sin(game.time.elapsed()) / 2 + 0.5, 1.0, 0.0);
      
              glVertex3f(0.5, 1.0, -6.0);
              glVertex3f(2.5, 1.0, -6.0);
      
              glColor3f(1.0, 1.0, sin(game.time.elapsed()) / 2 + 0.5);
      
              glVertex3f(2.5, -1.0, -6.0);
              glVertex3f(0.5, -1.0, -6.0);
          }
          glEnd();
      }

    });

    return EXIT_SUCCESS;
}
