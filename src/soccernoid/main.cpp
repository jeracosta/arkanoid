#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <cxxabi.h>
#include <format>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <print>
#include <random>
#include <typeinfo>

#include "oh-my-engine/game.hpp"
#include "oh-my-engine/input.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/gravity_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/shared_from.hpp"
#include "soccernoid/actions.hpp"
#include "soccernoid/nodes/root.hpp"

using namespace ome;
using namespace ome::ecs;
using namespace soccernoid;

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

class DespawnSystem : public System
{
  public:
    struct Configuration
    {
        float despawn_distance;
    } config;

    DespawnSystem(Configuration config)
        : config(config)
    {
    }

    void
    update(ecs::Entity &entity, Game &) override
    {
        auto &position = entity.get<components::Position>()->vector;

        if (norm(position) >= config.despawn_distance)
        {
            entity.kill();
        }
    }
};

class BounceSystem : public System
{
  public:
    struct Configuration
    {
        float elasticity;
        float speed_treshold = 0.4f;

        float floor_height = 0.0f;

    } config;

    BounceSystem(Configuration config)
        : config(config)
    {
    }

    void
    update(ecs::Entity &entity, Game &) override
    {
        auto &position = entity.get<components::Position>()->vector;
        auto &velocity = entity.get<components::Velocity>()->vector;
        auto &radius   = entity.get<components::HitSphere>()->radius;

        // HACK: hardcoded ad-hoc
        if (std::abs(position[0]) >= 1 || std::abs(position[2]) >= 1)
        {
            return;
        }

        Vec3f normal = up;

        float height = dot(position, normal) - config.floor_height;

        if (height >= radius)
        {
            return;
        }

        position += normal * (config.floor_height + radius - dot(position, normal));

        float velocity_at_normal = dot(velocity, normal);

        Vec3f opposing = normal * velocity_at_normal;

        if (std::abs(velocity_at_normal) < config.speed_treshold)
        {
            velocity -= opposing;
            return;
        }

        velocity -= opposing * (1.0f + config.elasticity);
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
        // Curve curve;
    } config;

    PauseColorSystem(Configuration config)
        : config(config)
    {
    }

    Color
    filter(Color color, Game &) const
    {
        // auto intensity     = game.time.unscaled.since(game.pause.paused_at()) * config.speed;
        // auto interpolation = ome::Interpolation(color.rgb(), grayscale(color).rgb(),
        // config.curve); auto faded         = Color::rgb(interpolation(intensity));
        //
        // return game.pause.is_paused() ? faded : color;

        return color;
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

template <class T>
std::string
type_name()
{
    int         status = 0;
    char       *p      = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    std::string s      = (status == 0 && p) ? p : typeid(T).name();
    std::free(p);
    return s;
}

class DespawningNode : public Node
{
  private:
    int counter_;

  public:
    DespawningNode(int counter = 5)
        : counter_(counter)
    {
    }

    virtual void
    on_mount_() override
    {
        log("Hello, world!");
    }

    virtual void
    on_tick_() override
    {
        log(std::format("{} ticks left", counter_), LogLevel::Debug);

        if (counter_-- == 0)
        {
            schedule_unmount();
        }
    }

    virtual void
    on_unmount_() override
    {
        log("Bye, bye!");
    }
};

class FallingNode : public Node
{
    void
    on_mount_() override
    {
        extending(*this).add<TransformNode>().add<GravityNode>();
    }
};

int
main()
{
    std::function<void(Color)> color_filter;

    struct
    {
        Vec3f from = { -3.0f, 3.0f, -3.0f };
        Vec3f to   = { 3.0f, 4.0f, 3.0f };
    } spawn_area;

    Vec3f *gravity;

    Game::run({
      .window = {
            .title = "Soccernoid",
            .size  = {640, 480},
            .resizeable = true,
      },

      .camera = {
          .distance = 3,
      },

      .make_input_mapper = [&](auto &game)
      {
          using namespace input;
          using enum KeyInput;

          auto inputs = InputMapper();

          inputs.bind(SDLK_ESCAPE, Release, Action::Quit);
          game.hold(inputs.bind(Action::Quit, [&]{ game.stop(); }));

          inputs.bind(SDLK_F10, Press, Action::ToggleFullscreen);
          game.hold(inputs.bind(Action::ToggleFullscreen, [&]
          {
              game.window.toggle_fullscreen();
          }));

          inputs.bind(SDLK_p, Press, Action::TogglePause);

          inputs.bind(SDLK_TAB, Press, Action::ChangeView);

          inputs.bind(SDLK_SPACE, {Press, Repeat}, Action::SummonBalls);
          game.hold(inputs.bind(Action::SummonBalls, [&] 
          {
              auto radius = 0.03f;

              auto spawn = [&] {

                  auto position = math::make_random_vector(spawn_area.from, spawn_area.to, rng);
                  auto color = Color::rgb(math::make_random_vector(Vec3f{0.5f}, Vec3f{1.0f}, rng));

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
              };

              constexpr uint spawn_rate = 20;

              for (auto _: std::views::iota(0u, spawn_rate))
              {
                  spawn();
              }

              std::println("Entity count: {}", game.entities.living_count());
          }));

          inputs.bind(SDLK_RETURN, {Press, Repeat}, Action::JiggleBalls);
          game.hold(inputs.bind(Action::JiggleBalls, [&]
          {
            for (auto entity: game.entities.living())
            {
                auto &velocity = entity.template get<components::Velocity>()->vector;
                velocity[1] += std::uniform_real_distribution<>{0, 5}(rng);
            }
          }));

          inputs.bind(SDLK_r, Press, Action::Reset);
          game.hold(inputs.bind(Action::Reset , [&]
          {
              game.entities.kill_all();
              std::println("Wiped entities");
          }));

          inputs.bind(SDLK_i, Press, Action::PrintInfo);
          game.hold(inputs.bind(Action::PrintInfo, [&]
          {
              std::println("Entities: {}", game.entities.living());
          }));

          inputs.bind(SDLK_PLUS,  { Press, Repeat}, Action::TimeSpeedUp);
          inputs.bind(SDLK_MINUS, { Press, Repeat}, Action::TimeSpeedDown);

          inputs.bind(SDLK_w,      Press, Action::CameraForward);
          inputs.bind(SDLK_s,      Press, Action::CameraBackward);
          inputs.bind(SDLK_a,      Press, Action::CameraLeft);
          inputs.bind(SDLK_d,      Press, Action::CameraRight);
          inputs.bind(SDLK_SPACE,  Press, Action::CameraUp);
          inputs.bind(SDLK_LCTRL,  Press, Action::CameraDown);
          inputs.bind(SDLK_LSHIFT, Press, Action::CameraSprint);

          game.hold(inputs.bind<MouseMotionInput>([&]
          {
              auto& camera = game.camera;

              *gravity = camera.down() * 9.81f;

              spawn_area.from = from_camera_space({ -0.5f, 3.0f, -camera.distance() - 0.5f }, camera);
              spawn_area.to   = from_camera_space({ 0.5f, 4.0f, -camera.distance() + 0.5f }, camera);
          }));

          return inputs;
      },

      .configure_systems = [&](auto &systems, auto &) {
          auto &fall_system = systems.push(FallSystem({}));
          systems.push(BounceSystem({
              .elasticity = 0.7f,
              .floor_height = 0.0f,
          }));
          systems.push(SphereRenderSystem{});
          systems.push(DespawnSystem({.despawn_distance = 5.0f}));
          // systems.push(DebugSystem{});

          gravity = &fall_system.config.gravity;

      },

      .make_root_node = [&] (Game &)
      {
          return std::make_shared<RootNode>();
      },

    .on_init = [](Game &)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    },

    .on_update = [&](auto &)
    {
        // cancha
        glBegin(GL_QUADS);
        {
            glColor(Color::rgb(0.1, 0.8, 0.1));
            glVertex3f(-1.0, 0.0, 1.0);
            glVertex3f(1.0, 0.0, 1.0);
            glVertex3f(1.0, 0.0, -1.0);
            glVertex3f(-1.0, 0.0, -1.0);
        }
        glEnd();

        // arco
        glBegin(GL_QUADS);
        {
            glColor(Color::rgb(0.9, 0.95, 0.85));
            glVertex3f(.25, 0.0, -1.0);
            glVertex3f(.25, 0.2, -1.0);
            glVertex3f(-.25, 0.2, -1.0);
            glVertex3f(-.25, 0.0, -1.0);
        }
        glEnd();
    }
});

    return EXIT_SUCCESS;
}
