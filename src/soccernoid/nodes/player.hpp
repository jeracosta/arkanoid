#include <GL/glu.h>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/nodes/kinematic_node.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/projectile.hpp"

namespace soccernoid {

class PlayerNode : public ome::KinematicNode
{
  public:
    struct Configuration
    {
        float movement_force;
        float max_speed;
        float speed_decay;

        static Configuration
        make_harry()
        {
            return {
                .movement_force = 10.0f,
                .max_speed      = 5.0f,
                .speed_decay    = 2.0f,
            };
        }
    };

  private:
    Configuration config_;

    void
    render_()
    {
        constexpr float         radius = 0.2f;
        static const ome::Color color  = ome::Color::hex(0x1486cc);

        auto position = transform<ome::Space::World>().position;

        glColor(color);
        glPushMatrix();
        {
            GLUquadric *q = gluNewQuadric();
            gluQuadricNormals(q, GLU_SMOOTH);
            glTranslatef(position[0], position[1], position[2]);
            gluSphere(q, radius, 32, 32);
            gluDeleteQuadric(q);
        }
        glPopMatrix();
    }

    void
    process_movement_()
    {
        velocity(kinematic().velocity * std::exp(-config_.speed_decay * game()->time.delta()));

        struct MoveSpecification
        {
            Action     action;
            ome::Vec3f direction;

            operator const ome::Vec3f &() const
            {
                return direction;
            }
        };

        static auto moves = std::to_array<MoveSpecification>({
            { Action::PlayerLeft, ome::left },
            { Action::PlayerRight, ome::right },
        });

        auto is_active = [this](const MoveSpecification &move)
        { return game()->input.is_pressed(move.action); };

        auto active_moves = moves | std::views::filter(is_active);

        auto raw_direction = std::ranges::fold_left(active_moves, ome::Vec3f{}, std::plus{});

        if (norm(raw_direction) == 0)
        {
            return;
        }

        auto direction = normalized(raw_direction);

        update_kinematic<ome::Space::World>([&](auto &k)
        {
            k.velocity += direction * config_.movement_force * game()->time.delta();
            if (norm(k.velocity) > config_.max_speed)
            {
                k.velocity = normalized(k.velocity) * config_.max_speed;
            }
        });
    }

    void
    shoot_()
    {
        auto &projectile = game()->root_node()->emplace_child<ProjectileNode>();

        projectile.position(transform<ome::Space::World>().position);
        projectile.velocity(0.2 * ome::up + 5.0 * ome::forward);
    }

  public:
    PlayerNode(const Configuration &config)
        : config_(config)
    {
    }

    void
    on_tick_() override
    {
        process_movement_();
        ome::KinematicNode::on_tick_();
        render_();
    }

    void
    on_mount_() override
    {
        update_transform<ome::Space::Local>([&](auto &t) { t.position = ome::up * 1.5f; });
        log("Hola");

        hold(game()->input.bind(Action::PlayerShoot, &PlayerNode::shoot_, this));
    }
};

} // namespace soccernoid
