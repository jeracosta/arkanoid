#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <print>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/game.hpp"
#include "oh-my-engine/math/functions.hpp"
#include "oh-my-engine/math/vector.hpp"

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
    auto view = CameraView::ThirdPerson;

    auto camera = make_camera(view);

    struct
    {
        float      speed = 1.5f;
        ome::Vec3f moving_direction{};
    } player;

    static constexpr auto gravity = ome::Vec3f(0.0f, -9.8f, 0.0f);

    struct Ball
    {
        ome::Vec3f position            = { 0.0f, 1.0f, 0.0f };
        ome::Vec3f speed               = { 0.0f, 0.2f, 0.0f };
        float      elasticity          = 0.75f;
        float      radius              = 0.05f;
        float      speed_stop_treshold = 0.4f;

        void
        update(float delta)
        {
            auto height       = position[1] - radius;
            auto acceleration = gravity;
            speed += acceleration * delta;
            position += speed * delta;

            if (height < 0.0f)
            {
                position[1] = radius;
                if (std::abs(speed[1]) < speed_stop_treshold)
                {
                    speed[1] = 0.0f;
                }
                else
                {
                    speed[1] = -speed[1] * elasticity;
                }
            }
        }

    } ball;

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
              game.toggle_pause();
              std::println("{}", game.is_paused() ? "Paused" : "Resumed");
          });

          inputs.keyboard.bind(SDLK_TAB, Press, [&]
          {
              view = next(view);
              camera = make_camera(view, camera);
              std::println("Switched to {} view", view == CameraView::FirstPerson ? "first person" : "third person");
          });

          inputs.keyboard.bind(SDLK_r, Press, [&]
          {
              ball = {};
              std::println("Ball reset");
          });

          inputs.keyboard.bind(SDLK_PLUS, Press, [&]
          {
              auto scale = game.time.scale();
              auto new_scale = scale * 1.5f;
              auto delta = new_scale - scale;
              game.time.scale(new_scale);
              std::println("Time scale: {} // {}{}", new_scale, delta > 0 ? "+" : "-", delta);
          });

          inputs.keyboard.bind(SDLK_MINUS, Press, [&]
          {
              auto scale = game.time.scale();
              auto new_scale = scale / 1.5f;
              auto delta = new_scale - scale;
              game.time.scale(new_scale);
              std::println("Time scale: {} // {}{}", new_scale, delta > 0 ? "+" : "-", delta);
          });

          using namespace ome::directions;

        // clang-format off
          #define BIND_MOVE(KEY, DIR)                                           \
              inputs.keyboard.bind(KEY, { Press, Release }, [&](auto input) {   \
                  auto dir = DIR;                                               \
                  player.moving_direction += (input == Press ? dir : -dir);     \
              })
          BIND_MOVE(SDLK_w, forward);
          BIND_MOVE(SDLK_s, backward);
          BIND_MOVE(SDLK_a, left);
          BIND_MOVE(SDLK_d, right);
          BIND_MOVE(SDLK_SPACE, up);
          BIND_MOVE(SDLK_LCTRL, down);
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
          if (view == CameraView::FirstPerson) {
              auto dir = camera.orientation.quat() * glm::vec3(player.moving_direction);
              camera.target += dir * player.speed * game.time.unscaled.delta();
          }

          gluLookAt(camera);

          auto color_filter = [&](ome::Vec3f color)
          {
              static constexpr auto speed = 7;
              auto intensity = game.time.since(game.pause_timestamp()) * speed;
              auto gray = grayscale(color);
              auto faded = ome::math::make_smoothstep(color, gray, 0.7)(intensity);
              return game.is_paused() ? faded : color;
          };

          auto set_color = [&](ome::Vec3f color)
          {
              auto filtered = color_filter(color);
              glColor3f(filtered[0], filtered[1], filtered[2]);
          };

          // cancha
          glBegin(GL_QUADS);
          {
              set_color({0.1, 0.8, 0.1});
              glVertex3f( -1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 , -1.0);
              glVertex3f( -1.0  , 0.0 , -1.0);
          }
          glEnd();

          // arco
          glBegin(GL_QUADS);
          {
              set_color({1.0, 1.0, 1.0});
              glVertex3f(  .25  , 0.0 , -1.0);
              glVertex3f(  .25  , 0.2 , -1.0);
              glVertex3f( -.25  , 0.2 , -1.0);
              glVertex3f( -.25  , 0.0 , -1.0);
          }
          glEnd();

          // pelota
          ball.update(game.time.delta());
          glPushMatrix();
          {
              GLUquadric* q = gluNewQuadric();
              glTranslatef(ball.position[0], ball.position[1], ball.position[2]);
              set_color({0.9, 0.2, 0.1});
              gluSphere(q, ball.radius, 32, 32);
              gluDeleteQuadric(q);
          }
          glPopMatrix();

      }
    });

    return EXIT_SUCCESS;
}
