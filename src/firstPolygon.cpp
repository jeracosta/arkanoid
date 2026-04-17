#include "game.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>

int
main()
{
    Game::run({
      .window = {
          .title = "Test SDL app",
          .size  = {640, 480},
      },

      .configure_input = [](auto &inputs, auto &game)
      {
          inputs.bind(SDLK_ESCAPE, KeyInput::Release, [&]{ game.stop(); });
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
      
          glTranslatef(-1.5f, 0.0f, -sin(game.time.elapsed()) / 0.01);
      
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
