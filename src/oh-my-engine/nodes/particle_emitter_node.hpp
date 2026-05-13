#pragma once

#include <functional>

#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

struct ParticleBlueprint
{
    struct RandomVec3f
    {
        Vec3f mean;
        Vec3f max_deviation = { 0.0f };

        Vec3f
        sample() const
        {
            static thread_local std::mt19937 rng{ std::random_device{}() };

            auto min = mean - max_deviation;
            auto max = mean + max_deviation;

            return make_random_vector(min, max, rng);
        }
    };

    std::function<Vec4f(float)> color;

    std::function<float(float)> scale;

    RandomVec3f origin;
    RandomVec3f initial_velocity;
    RandomVec3f acceleration;

    std::function<float(float)> angular_speed;
    float                       time_to_live;
};

template <std::size_t TCapacity>
class ParticleServer
{
  private:
    ParticleBlueprint blueprint_;

    struct Particle
    {
        Vec3f position;
        Vec3f velocity;
        Vec3f acceleration;
        float angle;
        float age;
        float time_to_live;

        Vec4f color;
        float scale;
        float angular_speed;
    };

    std::array<Particle, TCapacity> particles_;
    std::size_t                     particle_count_ = 0;

    void
    kill_(std::size_t i)
    {
        --particle_count_;
        if (i != particle_count_)
        {
            particles_[i] = particles_[particle_count_];
        }
    }

  public:
    explicit ParticleServer(ParticleBlueprint blueprint)
        : blueprint_(std::move(blueprint))
    {
    }

    void
    emit()
    {
        if (particle_count_ >= TCapacity)
        {
            return;
        }

        auto i = particle_count_++;

        float t = 0.0f;

        particles_[i] = Particle{ .position      = blueprint_.origin.sample(),
                                  .velocity      = blueprint_.initial_velocity.sample(),
                                  .acceleration  = blueprint_.acceleration.sample(),
                                  .angle         = 0.0f,
                                  .age           = 0.0f,
                                  .time_to_live  = blueprint_.time_to_live,
                                  .color         = blueprint_.color(t),
                                  .scale         = blueprint_.scale(t),
                                  .angular_speed = blueprint_.angular_speed(t) };
    }

    void
    emit(std::size_t count)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            emit();
        }
    }

    void
    update(float dt)
    {
        for (std::size_t i = 0; i < particle_count_;)
        {
            auto &p = particles_[i];

            p.age += dt;
            if (p.age >= p.time_to_live)
            {
                kill_(i);
                continue;
            }

            float t = p.age / p.time_to_live;

            p.velocity += p.acceleration * dt;
            p.position += p.velocity * dt;

            p.angle += blueprint_.angular_speed(t) * dt;

            p.color = blueprint_.color(t);

            p.scale = blueprint_.scale(t);

            ++i;
        }
    }

    void
    render(ome::Vec3f camera_up, ome::Vec3f camera_right) const
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // TODO: Make configurable
        glDepthMask(GL_FALSE);

        glBegin(GL_QUADS);
        for (std::size_t i = 0; i < particle_count_; ++i)
        {
            const auto &p      = particles_[i];
            auto        half_u = camera_right * (p.scale * 0.5f);
            auto        half_v = camera_up * (p.scale * 0.5f);

            glColor4f(p.color[0], p.color[1], p.color[2], p.color[3]);

            auto vertex = p.position - half_u - half_v;
            glVertex3f(vertex[0], vertex[1], vertex[2]);

            vertex = p.position + half_u - half_v;
            glVertex3f(vertex[0], vertex[1], vertex[2]);

            vertex = p.position + half_u + half_v;
            glVertex3f(vertex[0], vertex[1], vertex[2]);

            vertex = p.position - half_u + half_v;
            glVertex3f(vertex[0], vertex[1], vertex[2]);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    std::size_t
    count() const
    {
        return particle_count_;
    }

    ParticleBlueprint *
    blueprint()
    {
        return &blueprint_;
    }
};

class ParticleEmitterNode : public TransformNode
{
  private:
    ParticleServer<1000> particles_;
    float                emission_accumulator_ = 0;
    float                emission_period_;
    float                use_unscaled_time_;

  public:
    struct Configuration
    {
        ParticleBlueprint particle_blueprint;
        float             emissions_per_time_unit_;
        bool              use_unscaled_time = false;
    };

    ParticleEmitterNode(Configuration config)
        : particles_(config.particle_blueprint),
          emission_period_(config.emissions_per_time_unit_ > 0.0f
                               ? 1.0f / config.emissions_per_time_unit_
                               : std::numeric_limits<float>::infinity()),
          use_unscaled_time_(config.use_unscaled_time)
    {
    }

    void
    on_tick_() override
    {
        const float delta_time
            = use_unscaled_time_ ? game()->time.unscaled.delta() : game()->time.delta();

        const Vec3f last_position    = blueprint()->origin.mean;
        const Vec3f current_position = world_transform().position;

        emission_accumulator_ += delta_time;

        const std::size_t emission_count
            = static_cast<std::size_t>(emission_accumulator_ / emission_period_);

        float elapsed   = 0.0f;
        float spawn_lag = delta_time + emission_period_ - emission_accumulator_;

        for (std::size_t i = 0; i < emission_count; ++i)
        {
            particles_.update(spawn_lag - elapsed);

            float tick_progress      = std::clamp(spawn_lag / delta_time, 0.0f, 1.0f);
            blueprint()->origin.mean = lerp(last_position, current_position, tick_progress);
            particles_.emit();

            elapsed = spawn_lag;
            spawn_lag += emission_period_;
        }

        particles_.update(delta_time - elapsed);

        auto &camera = Node::game()->camera;

        game()->schedule([this, up = camera.up(), right = camera.right()]
        { particles_.render(up, right); });

        blueprint()->origin.mean = current_position;
        emission_accumulator_ -= static_cast<float>(emission_count) * emission_period_;
    }

    std::size_t
    particle_count()
    {
        return particles_.count();
    }

    ParticleBlueprint *
    blueprint()
    {
        return particles_.blueprint();
    }
};
} // namespace ome
