#pragma once

#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/result_type.hpp>
#include <concepts>
#include <functional>
#include <limits>
#include <random>

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/region.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"

namespace ome {

// #region Particle data

struct ParticleData
{
    Vec3f position;
    Vec3f velocity;
    Vec3f acceleration;
    float angle;
    float age;
    float time_to_live;

    Color color;
    float scale;
    float angular_speed;

    float
    progress() const
    {
        return age / time_to_live;
    }
};

// #endregion

// #region Particle scheme

struct ParticleScheme
{

    // #region Particle scheme item

    template <class Sig>
    class Item;

    template <class Return, class... Args>
    class Item<Return(Args...)>
    {
      private:
        using Function_                     = std::function<Return(Args...)>;
        static constexpr std::size_t arity_ = sizeof...(Args);

        template <class G>
        static constexpr bool invocable_as_ = std::is_constructible_v<Function_, G>;

        static constexpr bool updates_particle_
            = (arity_ == 1) && (std::same_as<std::remove_cvref_t<Args>, ParticleData> && ...);

      public:
        Item() = default;
        Item(std::nullptr_t)
        {
        }

        explicit Item(Function_ f)
            : fn_(std::move(f))
        {
        }

        template <class F>
            requires invocable_as_<F &&>
        Item(F &&f)
            : fn_(std::forward<F>(f))
        {
        }

        template <class Fn>
            requires(!invocable_as_<Fn &&> && std::is_invocable_r_v<Return, Fn &>)
        Item(Fn &&f)
            : fn_([f = std::forward<Fn>(f)](Args...) mutable -> Return { return f(); })
        {
        }

        template <class Value>
            requires(arity_ == 0 && std::convertible_to<Value, Return> && !invocable_as_<Value &>)
        Item(Value &&value)
            : fn_([value = Return(std::forward<Value>(value))](Args...) -> Return { return value; })
        {
        }

        template <class Value>
            requires(arity_ == 1 && std::convertible_to<Value, Return> && !invocable_as_<Value &>)
        Item(Value &&value)
            : fn_([value = Return(std::forward<Value>(value))](Args...) -> Return { return value; })
        {
        }

        template <class TCurve>
            requires updates_particle_
                     && std::derived_from<std::remove_cvref_t<TCurve>, Curve<Return>>
        Item(TCurve &&curve)
            : fn_([curve = std::forward<TCurve>(curve)](const ParticleData &particle) -> Return
        { return curve(particle.progress()); })
        {
        }

        template <math::Region TRegion, class Rng>
        Item(TRegion &&region, Rng &rng)
            : fn_([region = std::forward<TRegion>(region), &rng]() -> Return
        { return region.sample_uniform(rng); })
        {
        }

        Return
        operator()(Args... args) const
        {
            return fn_(std::forward<Args>(args)...);
        }

      private:
        Function_ fn_;
    };

    // #endregion

    // Values fixed at emission time
    Item<Vec3f()> initial_position = [] { return Vec3f{ 0 }; };
    Item<Vec3f()> initial_velocity = [] { return Vec3f{ 0 }; };
    Item<float()> angular_speed    = [] { return 0; };
    Item<float()> time_to_live     = [] { return 1; };

    // Values computed every frame
    Item<Vec3f(const ParticleData &)> acceleration = [](auto &) { return Vec3f{ 0 }; };
    Item<Color(const ParticleData &)> color        = [](auto &) { return Color::white(); };
    Item<float(const ParticleData &)> scale        = [](auto &) { return 1; };
};

// #endregion

template <std::size_t TCapacity>
class ParticleServer
{
  private:
    ParticleScheme scheme_;

    std::array<ParticleData, TCapacity> particles_;
    std::size_t                         live_particle_count_ = 0;

    Vec3f emission_origin_; // particle initial position is relative to this point

    void
    kill_(std::size_t i)
    {
        --live_particle_count_;
        if (i != live_particle_count_)
        {
            particles_[i] = particles_[live_particle_count_];
        }
    }

  public:
    explicit ParticleServer(ParticleScheme blueprint, Vec3f emission_origin)
        : scheme_(std::move(blueprint)),
          emission_origin_(emission_origin)
    {
    }

    const Vec3f
    origin() const
    {
        return emission_origin_;
    }

    void
    emit()
    {
        if (live_particle_count_ >= TCapacity)
        {
            return;
        }

        auto i = live_particle_count_++;

        ParticleData particle{};
        particle.age           = 0.0f;
        particle.time_to_live  = scheme_.time_to_live();
        particle.position      = scheme_.initial_position() + emission_origin_;
        particle.velocity      = scheme_.initial_velocity();
        particle.acceleration  = scheme_.acceleration(particle);
        particle.color         = scheme_.color(particle);
        particle.scale         = scheme_.scale(particle);
        particle.angle         = 0.0f;
        particle.angular_speed = scheme_.angular_speed();

        particles_[i] = particle;
    }

    void
    emit(std::size_t count)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            emit();
        }
    }

    struct Update
    {
        float delta_time;
        Vec3f new_origin;
    };

    void
    update(const Update &update)
    {
        const Vec3f origin_delta = update.new_origin - emission_origin_;

        for (std::size_t i = 0; i < live_particle_count_;)
        {
            auto &p = particles_[i];

            p.age += update.delta_time;
            if (p.age >= p.time_to_live)
            {
                kill_(i);
                continue;
            }

            p.acceleration = scheme_.acceleration(p);
            p.velocity += p.acceleration * update.delta_time;
            p.position += origin_delta + p.velocity * update.delta_time;
            p.angle += p.angular_speed * update.delta_time;
            p.color = scheme_.color(p);
            p.scale = scheme_.scale(p);

            ++i;
        }

        emission_origin_ = update.new_origin;
    }

    void
    render(ome::Vec3f camera_up, ome::Vec3f camera_right) const
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // TODO: Make configurable
        glDepthMask(GL_FALSE);

        glBegin(GL_QUADS);
        for (std::size_t i = 0; i < live_particle_count_; ++i)
        {
            const auto &p      = particles_[i];
            auto        half_u = camera_right * (p.scale * 0.5f);
            auto        half_v = camera_up * (p.scale * 0.5f);

            glColor(p.color);

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
        return live_particle_count_;
    }

    ParticleScheme *
    blueprint()
    {
        return &scheme_;
    }
};

class ParticleEmitterNode : public TransformNode
{
  public:
    struct Settings
    {
        ParticleScheme particle_blueprint;
        float          emission_rate_; // how many particles to emit per time unit
        bool           use_unscaled_time = false;
    };

    ParticleEmitterNode(Settings settings)
        : particles_(settings.particle_blueprint, world_transform().position),
          settings_(std::move(settings))
    {
    }

    void
    on_tick_() override
    {
        const float delta_time
            = settings_.use_unscaled_time ? game()->time.unscaled.delta() : game()->time.delta();

        const auto previous_origin = particles_.origin();
        const auto current_origin  = world_transform().position;

        const float period = settings_.emission_rate_ > 0.0f
                                 ? 1.0f / settings_.emission_rate_
                                 : std::numeric_limits<float>::infinity();

        [[unlikely]]
        if (!std::isfinite(period))
        {
            particles_.update({ delta_time, current_origin });
        }
        else
        {
            const float time_since_last_emit = accumulator_;
            float       cursor               = 0.0f;
            float       next_emit_at         = period - time_since_last_emit;

            while (next_emit_at <= delta_time)
            {
                const float segment  = next_emit_at - cursor;
                const float progress = delta_time > 0.0f ? next_emit_at / delta_time : 1.0f;
                const Vec3f origin   = lerp(previous_origin, current_origin, progress);

                particles_.update({ segment, origin });
                particles_.emit();

                cursor = next_emit_at;
                next_emit_at += period;
            }

            if (cursor < delta_time)
            {
                particles_.update({ delta_time - cursor, current_origin });
            }

            accumulator_ = std::fmod(time_since_last_emit + delta_time, period);
        }

        auto &camera = Node::game()->camera;
        game()->schedule([this, up = camera.up(), right = camera.right()]
        { particles_.render(up, right); });
    }

    std::size_t
    particle_count()
    {
        return particles_.count();
    }

    ParticleScheme *
    blueprint()
    {
        return particles_.blueprint();
    }

  private:
    ParticleServer<1000> particles_;
    Settings             settings_;

    // Used to track leftover time between ticks for emission timing
    float accumulator_ = 0.0f;

  protected:
    static inline std::mt19937 rng = {};
};
} // namespace ome
