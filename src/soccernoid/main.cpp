#include "oh-my-engine/game.hpp"
#include "oh-my-engine/math/functions.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <print>

static float epsilon      = std::numeric_limits<float>::epsilon();
static float constexpr pi = std::numbers::pi_v<float>;

struct Camera
{
    glm::vec3 target;      // point you look at
    glm::quat orientation; // rotation around target
    float     distance;    // zoom
} camera;

void
render_camera(const Camera &camera)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);

    glm::vec3 forward(0.0f, 0.0f, camera.distance);

    glm::vec3 position = camera.target + (camera.orientation * forward);

    glm::vec3 up = camera.orientation * glm::vec3(0.0f, 1.0f, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(position.x,
              position.y,
              position.z,
              camera.target.x,
              camera.target.y,
              camera.target.z,
              up.x,
              up.y,
              up.z);
}

int
main()
{
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
              auto mouse_coords = ome::normalize(input.position, game.window);

              auto compute_angle = [&](float proportion, auto &interval) {
                  auto &[from, to] = interval;
                  auto sigmoid = ome::math::make_sigmoid(5.0f, 0.5f);
                  auto lerp = ome::math::make_lerp(from, to);
                  return lerp(sigmoid(proportion));
              };

              glm::quat yaw = glm::angleAxis(dx * sensitivity, glm::vec3(0,1,0));


              auto azimuth = compute_angle(mouse_coords[0], camera.angle.azimuth);
              auto inclination = compute_angle(mouse_coords[1], camera.angle.inclination);

              using enum ome::math::CoordinateSystem;
              auto position = ome::Vec3f::Spherical{ camera.distance, inclination, azimuth }.rebased<Cartesian>();

              update_camera(position);

          });
      },

      .on_update = [](auto &)
      {
          glClear(GL_COLOR_BUFFER_BIT);
          glClear(GL_DEPTH_BUFFER_BIT);
      
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
      
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
