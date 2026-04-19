#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <print>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/game.hpp"

// TODO: Que el cambio de view de third a first sea seamless.

ome::Vec3f
grayscale(ome::Vec3f color)
{
    return { dot(color, ome::Vec3f(0.299, 0.587, 0.114)) };
}

enum class CameraView
{
    FirstPerson,
    ThirdPerson,
    Count_
};

CameraView
next(CameraView view)
{
    return static_cast<CameraView>((static_cast<int>(view) + 1)
                                   % static_cast<int>(CameraView::Count_));
}

ome::Camera
make_camera(CameraView view, ome::Camera camera = {})
{
    switch (view)
    {
    case CameraView::FirstPerson:
    {
        auto distance      = 0.1f;
        auto delta_distace = camera.distance - distance;
        auto delta_target  = camera.orientation.backward() * delta_distace;
        return {
            .target      = camera.target + delta_target,
            .orientation = camera.orientation,
            .distance    = distance,
        };
    }
    case CameraView::ThirdPerson:
    {
        return {
            .distance = 5,
        };
    }
    default:
        throw std::runtime_error("Unsuported camera view");
    }
}

int
main()
{
    auto camera = make_camera(CameraView::ThirdPerson);

    auto view = [](auto &game)
    {
        using enum CameraView;
        return game.time.is_paused() ? FirstPerson : ThirdPerson;
    };

    struct
    {
        float      speed = 1.5f;
        ome::Vec3f moving_direction{};
    } player;

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

          inputs.keyboard.bind(SDLK_p, Press, [&]
          {
              game.time.toggle_pause();
              camera = make_camera(view(game), camera);
          });

          using namespace ome::directions;

        // clang-format off
          #define BIND_MOVE(KEY, DIR)                                           \
              inputs.keyboard.bind(KEY, { Press, Release }, [&](auto input) {   \
                  auto dir = camera.orientation.quat() * glm::vec3(DIR);        \
                  player.moving_direction += (input == Press ? dir : -dir);     \
              })
          BIND_MOVE(SDLK_w, forward);
          BIND_MOVE(SDLK_s, backward);
          BIND_MOVE(SDLK_a, left);
          BIND_MOVE(SDLK_d, right);
        // clang-format on

          inputs.mouse_motion.bind([&](auto input)
          {
              camera.orientation.steer_yaw(-input.delta[0] * 0.01f);
              camera.orientation.steer_pitch(-input.delta[1] * 0.01f);
          });
      },

      .on_init = []
      {
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
      },

      .on_update = [&](auto & game)
      {
          if (view(game) == CameraView::FirstPerson) {
              camera.target += player.moving_direction * player.speed * game.time.delta();
          }

          gluLookAt(camera);

          auto color_filter = [&](ome::Vec3f color)
          {
              return game.time.is_paused() ? grayscale(color) : color;
          };

          glBegin(GL_QUADS);
          {
              auto color = color_filter({0.1, 0.8, 0.3});
              glColor3f(color[0], color[1], color[2]);
              glVertex3f( -1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 , -1.0);
              glVertex3f( -1.0  , 0.0 , -1.0);
          }
          glEnd();

          glBegin(GL_QUADS);
          {
              auto color = color_filter({1.0, 1.0, 1.0});
              glColor3f(color[0], color[1], color[2]);
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
