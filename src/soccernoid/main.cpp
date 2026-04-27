#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <cxxabi.h>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <print>
#include <random>
#include <typeinfo>

#include "oh-my-engine/camera.hpp"
#include "oh-my-engine/game.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/curve.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/gravity_node.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

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

void
print_message(const auto &node, auto &&message)
{
    auto now  = std::chrono::system_clock::now();
    auto time = floor<std::chrono::seconds>(now);

    std::print("\033[37m[{:%H:%M:%S}]\033[0m "
               "\033[34m{} ({}): \033[0m "
               "\033[37m{}\033[0m\n",
               time,
               node.name(),
               type_name<std::remove_cvref_t<decltype(node)>>(),
               message);
}

class TestNode : public Node
{
  private:
    int counter_;

  public:
    TestNode(int counter = 5)
        : counter_(counter)
    {
    }

    virtual void
    on_mount_() override
    {
        print_message(*this, "Hello, world!");
    }

    virtual void
    tick_() override
    {
        print_message(*this, std::format("{} ticks left", counter_));

        if (counter_-- == 0)
        {
            die_();
        }
    }

    virtual void
    on_unmount_() override
    {
        print_message(*this, "Bye, bye!");
        std::println("Updated tree:");
        print_tree(find_root(this));
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

class FrameRateNode : public Node
{
  public:
    void
    tick_() override
    {
        print_message(*this, std::format("FPS: {}", game()->instant_frame_rate()));
    }
};

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
          });

          inputs.keyboard.bind(SDLK_RETURN, {Press, Repeat}, [&]
          {
            for (auto entity: game.entities.living())
            {
                auto &velocity = entity.template get<components::Velocity>()->vector;
                velocity[1] += std::uniform_real_distribution<>{0, 5}(rng);
            }
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

              *gravity = camera.orientation.down() * 9.81f;

              spawn_area.from = camera.to_world({ -0.5f, 3.0f, -camera.distance - 0.5f });
              spawn_area.to   = camera.to_world({ 0.5f, 4.0f, -camera.distance + 0.5f });

          });
      },

      .configure_systems = [&](auto &systems, auto &game) {
          auto &fall_system = systems.push(FallSystem({}));
          systems.push(BounceSystem({
              .elasticity = 0.7f,
              .floor_height = 0.0f,
          }));
          auto &pause_system = systems.push(PauseColorSystem({ .speed = 3.0f, .curve = pause_curve }));
          systems.push(SphereRenderSystem{});
          systems.push(DespawnSystem({.despawn_distance = 5.0f}));
          // systems.push(DebugSystem{});

          color_filter = [&](Color color)
          {
              return pause_system.filter(color, game);
          };

          gravity = &fall_system.config.gravity;

      },

      .make_root_node = [] (Game &)
      {
          auto root = std::make_unique<Node>("Root");

          auto print_height = [](FallingNode &node) {
              auto transform = find_descendant<TransformNode>(&node);
              auto height = math::dot(transform->position, up);
              print_message(node, std::format("Height: {}", height));
          };

          extending(*root)
              .add<Node>().named("Manolito")
                  .add<Node>().named("Fede")
                  .up()
              .up()
              .add<Slowed<TestNode, 0.5f>>(10).named("Prueba")
                  .add<Node>().named("Jaimito")
                  .up()
              .up()
              .add<Node>()
                  .add<FallingNode>().named("Falling")//.on_tick(print_height)
                  .up()
                  .add<Slowed<FrameRateNode, 1.0f>>().named("FPS")
                  .up()
                  .add<Node>().named("Marujita");

          return root;
      },

      .on_init = [](Game &game)
      {
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();
          gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);

          std::println("Game initialized. Tree:");
          print_tree(*game.root_node());
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
