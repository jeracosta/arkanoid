#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <print>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/game.hpp"

static float epsilon = std::numeric_limits<float>::epsilon();

int
main()
{
    auto camera = ome::Camera{
        .distance = 3,
    };

    ome::game::run({
      .window = {
            .title = "Soccernoid",
            .size  = {640, 480},
      },

      .configure_input = [&](auto &inputs, auto &game)
      {
          using enum ome::input::KeyInput;

          inputs.keyboard.bind(SDLK_ESCAPE, Release, [&]{ game.stop(); });

          inputs.keyboard.bind(SDLK_F10, Press, [&]
          {
              auto size = game.window.size();

              std::println("Toggling fullscreen mode. Current size: {} x {}", size[0], size[1]);

              game.window.toggle_fullscreen();
          });

          inputs.mouse_motion.bind([&](auto input)
          {
              camera.orientation.steer_yaw(-input.delta[0] * 0.01f);
              camera.orientation.steer_pitch(input.delta[1] * 0.01f);
          });
      },

      .on_init = []
      {
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
      },

      .on_update = [&](auto &)
      {
          gluLookAt(camera);

          glBegin(GL_QUADS);
          {
              glColor3f(0.1, 0.8, 0.3);
              glVertex3f( -1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 , -1.0);
              glVertex3f( -1.0  , 0.0 , -1.0);
          }
          glEnd();

          glBegin(GL_QUADS);
          {
              glColor3f(1,1,1);
              glVertex3f(  .25  , 0.0 , -1.0);
              glVertex3f(  .25  , 0.2 , -1.0);
              glVertex3f( -.25  , 0.2 , -1.0);
              glVertex3f( -.25  , 0.0 , -1.0);
          }
          glEnd();

      }
    });

    return EXIT_SUCCESS;
}
