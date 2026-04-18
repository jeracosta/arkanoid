#include "oh-my-engine/game.hpp"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <limits>
#include <print>

glm::vec2
circle_slice_parametric(float t, float from, float to, float radius)
{
    assert(t >= 0.0f && t <= 1.0f);
    assert(from >= 0.0f && from <= 1.0f);
    assert(to >= 0.0f && to <= 1.0f);

    t = 1 - t;
    t = (to - from) * t + from;
    t *= 2 * std::numbers::pi_v<float>;
    return { std::sin(t) * radius, std::cos(t) * radius };
}

static float epsilon = std::numeric_limits<float>::epsilon();

enum class CoordinateSystem
{
    Cartesian,
    Spherical
};

template <CoordinateSystem From, CoordinateSystem To>
glm::vec3
transform(glm::vec3 point);

inline float
sigmoid(float x, float steepness = 1.0f, float midpoint = 0.0f)
{
    return 1.0f / (1.0f + std::exp(-steepness * (x - midpoint)));
}

void
update_camera(glm::vec3 position)
{
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluPerspective(45, (640.0 / 480.0), 0.1, 100);

    // clang-format off
    gluLookAt(position.x, position.y, position.z,
              0.0, 0.0, 0.0,  // Look at point
              0.0, 1.0, 0.0); // Up vector
    // clang-format on
}

int
main()
{
    struct
    {
        float distance = 2.5;

        struct
        {
            struct Range
            {
                float from, to;
            };

            Range longitude, latitude;

        } angle{ { .01, .25f - epsilon }, { -0.05, 0.05 } };

    } camera;

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

              std::println("Toggling fullscreen mode. Current size: {} x {}", size.x, size.y);

              game.window.toggle_fullscreen();
          });

          inputs.mouse_motion.bind([&](auto mouse)
          {
              auto longitude = [&] {
                  auto t = static_cast<float>(mouse.position.y) / game.window.size().y;
                  t = sigmoid(t, 5.0f, 0.5f);
                  auto &[from, to] = camera.angle.longitude;
                  return glm::vec3{0, circle_slice_parametric(t, from, to, camera.distance)};
              }();

              auto latitude = [&] {
                  auto t = static_cast<float>(mouse.position.x) / game.window.size().x;
                  t = 1 - t;
                  t = sigmoid(t, 5.0f, 0.5f);
                  auto &[from, to] = camera.angle.latitude;
                  auto param = circle_slice_parametric(t, from, to, camera.distance);
                  return glm::vec3{param.x, 0, param.y};
              }();

              // FIXME: coords should be sumed in Spherical space, not Cartesian
              // TODO: circle_slice_parametric could be replaced for linear motion in that Spherical space
              update_camera(longitude /*+ latitude*/);
          });
      },

      .on_init = [&]
      {
          glClearColor(0.0,0.0,0.0,1.0); // Negro
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
