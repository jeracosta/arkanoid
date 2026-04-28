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
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/curve.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/node.hpp"
#include "oh-my-engine/nodes/gravity_node.hpp"
#include "oh-my-engine/nodes/mixins/eventful.hpp"
#include "oh-my-engine/nodes/mixins/slowed.hpp"
#include "oh-my-engine/nodes/particle_emitter_node.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/shared_from.hpp"
#include "soccernoid/level.hpp"
#include "soccernoid/nodes/camera_control.hpp"

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
        log(*this, "Hello, world!", game()->logger());
    }

    virtual void
    tick_() override
    {
        log(*this, std::format("{} ticks left", counter_), game()->logger(), LogLevel::Debug);

        if (counter_-- == 0)
        {
            die_();
        }
    }

    virtual void
    on_unmount_() override
    {
        log(*this, "Bye, bye!", game()->logger());
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

struct FrameRateEvent
{
    float frame_rate;
};

class FrameRateNode : public Eventful<Node, FrameRateEvent>
{
  public:
    void
    tick_() override
    {
        auto frame_rate = game()->instant_frame_rate();

        log(*this, std::format("FPS: {}", frame_rate), game()->logger());

        emit_(FrameRateEvent{ frame_rate });
    }
};

class FrameRateObserverNode : public Slowed<DespawningNode, 1.0f>
{
  public:
    FrameRateObserverNode()
        : Slowed<DespawningNode, 1.0f>(5) // 5 tics de vida
    {
    }

    void
    on_mount_() override
    {
        DespawningNode::on_mount_();

        auto callback = [this](const FrameRateEvent &event)
        {
            auto message = std::format("Observed FPS: {}", event.frame_rate);
            log(*this, message, game()->logger(), LogLevel::Debug);
        };

        auto connection = find_ancestor<FrameRateNode>(this)->bind(callback);
        hold(connection);
    }
};

using namespace soccernoid;

int
main()
{
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

    auto camera_node = std::make_shared<CameraControlNode>();

    auto test_connection = std::make_shared<EventConnection>();

    Game::run({
      .window = {
            .title = "Soccernoid",
            .size  = {640, 480},
      },

      .camera = {
          .distance = 3,
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
              using enum CameraView;

              auto view = succesor(camera_node->current_view());

              camera_node->set_view(view);

              auto message = std::format("Switched to {} view", view == CameraView::FirstPerson ? "first person" : "third person");
              log(*camera_node, message, game.logger());
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
              auto& camera = game.camera;

              float yaw   = -input.delta[0] * 0.01f;
              float pitch = -input.delta[1] * 0.01f;

              switch (camera_node->current_view())
              {
                case CameraView::FirstPerson:
                {
                    camera.steer_yaw(yaw);
                    camera.steer_pitch(pitch);
                    break;
                }
                case CameraView::ThirdPerson:
                {
                    // FIXME: it tilts
                    camera.rotate(yaw, camera.up());
                    camera.rotate(pitch, camera.right());
                    break;
                }
                default:
                    throw std::runtime_error("Unsuported camera view");
              }

              *gravity = camera.down() * 9.81f;

              spawn_area.from = from_camera_space({ -0.5f, 3.0f, -camera.distance() - 0.5f }, camera);
              spawn_area.to   = from_camera_space({ 0.5f, 4.0f, -camera.distance() + 0.5f }, camera);

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

      .make_root_node = [&] (Game &)
      {
          auto root = std::make_shared<Node>("Root");

          root->hold(root->bind<NodeGotReady>([&root = *root]{
              log(root, "¡Si capitán, estamos listos!", root.game()->logger());
              print_tree(root);
          }));

          ParticleBlueprint particle_blueprint = {
              .color = {
                  .origin = {1.0, 0.0, 0.0, 1.0},
                  .target = {0.0, 1.0, 0.5, 0.9},
                  .curve  = Curve::linear(),
              },
              .scale = {
                  .origin = 1.2f,
                  .target = 1.3f,
                  .curve  = Curve::linear(),
              },
              .velocity = {
                  .origin = Vec3f(0.0, -0.2, 0.0),
                  .target = Vec3f(0.0, -0.2, 0.2),
                  .curve  = Curve::linear(),
              },
              .origin = Vec3f(0.0, 0.2, 0.0),
              .angular_speed = {
                  .origin = 0,
                  .target = 0,
                  .curve  = Curve::linear(),
              },
              .time_to_live = 1.5,
          };

          auto particle_node = std::make_shared<ParticleEmitterNode>(ParticleEmitterNode::Configuration{
              .particle_blueprint = particle_blueprint,
              .emission_rate = 0.001f
          });

          extending(*root)
              .add(camera_node).named("Camera")
              .add<Node>().named("Manolito")
                  .add<Node>().named("Fede")
                  .up()
              .up()
              .add<Node>().named("Jaimito")
                  .add(particle_node).named("Particulas")
                  .up()
              .up()
              .add<Node>()
                  .add<FallingNode>().named("Falling")
                  .up()
                  .add<Slowed<FrameRateNode, 1.0f>>().named("FPS")
                      .add<FrameRateObserverNode>().named("Observador");

          return root;
      },

    .on_init =
        [](Game &game)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    },

    .on_update = [&](auto &game)
{
    if (camera_node->current_view() == CameraView::FirstPerson)
    {
        auto dir = game.camera.orientation().quat() * glm::vec3(player.moving_direction);
        game.camera.target(game.camera.target() + dir * player.speed * game.time.unscaled.delta());
    }

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
