#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <print>
#include <random>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/game.hpp"
#include "oh-my-engine/math/vector.hpp"

using namespace ome;
using namespace ome::ecs;

static auto rng = std::mt19937{ std::random_device{}() };

class FallSystem : public System
{
  public:
    struct Configuration
    {
        Vec3f gravity = { 0.0f, -9.81f, 0.0f };
    } config;

    FallSystem(Configuration config)
        : config(config)
    {
    }

    void
    update(ecs::Entity &entity, Game &game) override
    {
        auto &velocity = entity.get<components::Velocity>()->vector;
        auto &position = entity.get<components::Position>()->vector;

        velocity += config.gravity * game.time.delta();
        position += velocity * game.time.delta();
    }
};

class BounceSystem : public System
{
  public:
    struct Configuration
    {
        float elasticity;
        float floor_height   = 0.0f;
        float speed_treshold = 0.4f;
    } config;

    BounceSystem(Configuration config)
        : config(config)
    {
    }

    void
    update(ecs::Entity &entity, Game &) override
    {
        auto &height = entity.get<components::Position>()->vector[1];
        auto &speed  = entity.get<components::Velocity>()->vector[1];
        auto &radius = entity.get<components::HitSphere>()->radius;

        if (height - radius < config.floor_height)
        {
            if (std::abs(speed) < config.speed_treshold)
            {
                height = config.floor_height + radius;
                speed  = 0.0f;
            }

            height = config.floor_height + radius;
            speed  = -speed * config.elasticity;
        }
    }
};

class SphereRenderSystem : public System
{
  public:
    void
    update(ecs::Entity &entity, Game &) override
    {
        auto &position = entity.get<components::Position>()->vector;
        auto  radius   = entity.get<components::HitSphere>()->radius;
        auto  color    = entity.get<components::Color>()->color;

        glColor(color);

        glPushMatrix();
        {
            GLUquadric *q = gluNewQuadric();
            glTranslatef(position[0], position[1], position[2]);
            gluSphere(q, radius, 32, 32);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }
};

class PauseColorSystem : public System
{
  public:
    struct Configuration
    {
        float speed;
        Curve curve;
    } config;

    PauseColorSystem(Configuration config)
        : config(config)
    {
    }

    Color
    filter(Color color, Game &game) const
    {
        auto intensity     = game.time.unscaled.since(game.pause.paused_at()) * config.speed;
        auto interpolation = ome::Interpolation(color.rgb(), grayscale(color).rgb(), config.curve);
        auto faded         = Color::rgb(interpolation(intensity));

        return game.pause.is_paused() ? faded : color;
    }

    void
    update(ecs::Entity &entity, Game &game) override
    {
        auto &color = entity.get<components::Color>()->color;
        color       = filter(color, game);
    }
};

class DebugSystem : public System
{
  public:
    void
    update(ecs::Entity &entity, Game &) override
    {
        auto &position = entity.get<components::Position>()->vector;

        std::println("Entity {} at position {}", entity, position);
    }
};

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

Camera
make_camera(CameraView view, Camera camera = {})
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
        float speed = 1.5f;
        Vec3f moving_direction{};
    } player;

    std::function<void(Color)> color_filter;

    auto pause_curve = ome::Curve::smoothstep(5.0f);

    Game::run({
        .window = {
            .title = "Soccernoid",
            .size  = {640, 480},
      },

      .configure_input = [&](auto &inputs, auto &game)
      {
          using enum input::KeyInput;

          inputs.keyboard.bind(SDLK_ESCAPE, Release, [&]{ game.stop(); });

          inputs.keyboard.bind(SDLK_F10, Press, [&]
          {
              auto size = game.window.size();

              std::println("Toggling fullscreen mode. Current size: {} x {}", size[0], size[1]);

              game.window.toggle_fullscreen();
          });

          inputs.keyboard.bind(SDLK_p, Press, [&]
          {
              game.pause.toggle();
              std::println("{}", game.pause.is_paused() ? "Paused" : "Resumed");
          });

          inputs.keyboard.bind(SDLK_TAB, Press, [&]
          {
              view = next(view);
              camera = make_camera(view, camera);
              std::println("Switched to {} view", view == CameraView::FirstPerson ? "first person" : "third person");
          });

          inputs.keyboard.bind(SDLK_SPACE, {Press, Repeat}, [&]
          {
              struct {
                Vec3f from = {-1.0f, 1.5f, -1.0f};
                Vec3f to   = { 1.0f, 3.0f,  1.0f};
               } spawn_area;

              auto position = math::make_random_vector(spawn_area.from, spawn_area.to, rng);
              auto color = Color::rgb(math::make_random_vector(Vec3f{0.5f}, Vec3f{1.0f}, rng));

              auto radius = 0.05f;

              if (game.entities.full())
              {
                  game.entities.random(rng).kill();
              }

              game.entities.emplace(
                  components::Position(position),
                  components::Velocity(Vec3f{}),
                  components::HitSphere(radius),
                  components::Color(color)
              );
          });

          inputs.keyboard.bind(SDLK_r, Press, [&]
          {
              game.entities.kill_all();
              std::println("Wiped entities");
          });

          inputs.keyboard.bind(SDLK_i, Press, [&]
          {
              std::println("Entities: {}", game.entities.living());
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

          using namespace directions;

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
              float yaw   = -input.delta[0] * 0.01f;
              float pitch = -input.delta[1] * 0.01f;

              switch (view)
              {
                case CameraView::FirstPerson:
                {
                    camera.orientation.steer_yaw(yaw);
                    camera.orientation.steer_pitch(pitch);
                    break;
                }
                case CameraView::ThirdPerson:
                {
                    camera.orientation.rotate(yaw, camera.orientation.down());
                    camera.orientation.rotate(pitch, camera.orientation.left());
                    break;
                }
                default:
                    throw std::runtime_error("Unsuported camera view");
              }
          });
      },

      .configure_systems = [&](auto &systems, auto &game) {
          systems.push(FallSystem({}));
          systems.push(BounceSystem({.elasticity = 0.7f}));
          auto &pause = systems.push(PauseColorSystem({ .speed = 3.0f, .curve = pause_curve }));
          systems.push(SphereRenderSystem{});
          // systems.push(DebugSystem{});

          color_filter = [&](Color color)
          {
              return pause.filter(color, game);
          };

      },

      .on_init = [](auto &)
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

          // cancha
          glBegin(GL_QUADS);
          {
              glColor(Color::rgb( 0.1, 0.8, 0.1 ));
              glVertex3f( -1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 ,  1.0);
              glVertex3f(  1.0  , 0.0 , -1.0);
              glVertex3f( -1.0  , 0.0 , -1.0);
          }
          glEnd();

          // arco
          glBegin(GL_QUADS);
          {
              glColor(Color::rgb( 0.9, 0.95, 0.85 ));
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
