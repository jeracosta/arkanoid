#pragma once

#include "oh-my-engine/color.hpp"
#include "oh-my-engine/curve.hpp"
#include "oh-my-engine/math/region.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/open_gl/blend_mode.hpp"

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

struct ParticleUpdateContext
{
    const ParticleData &particle;
    const float        &delta_time;
};

struct ParticleScheme
{
    // #region Item

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

        // Receives particle data and delta time, returns a value to update the particle with.
        static constexpr bool updates_particle_
            = std::is_invocable_r_v<Return, Function_, const ParticleUpdateContext &>;

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
            : fn_([curve
                   = std::forward<TCurve>(curve)](const ParticleUpdateContext &context) -> Return
        { return curve(context.particle.progress()); })
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
    Item<Vec3f(const ParticleUpdateContext &)> acceleration = [](auto &) { return Vec3f{ 0 }; };
    Item<Color(const ParticleUpdateContext &)> color        = [](auto &) { return Color::white(); };
    Item<float(const ParticleUpdateContext &)> scale        = [](auto &) { return 1; };

    // [0, 1] multiplier for displacement towards emitter position
    float emitter_pull = 0;

    open_gl::BlendMode blend_mode = {};
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

        auto particle = ParticleData{};

        auto context = ParticleUpdateContext{ .particle = particle, .delta_time = 0.0f };

        particle.age           = 0.0f;
        particle.time_to_live  = scheme_.time_to_live();
        particle.position      = scheme_.initial_position() + emission_origin_;
        particle.velocity      = scheme_.initial_velocity();
        particle.acceleration  = scheme_.acceleration(context);
        particle.color         = scheme_.color(context);
        particle.scale         = scheme_.scale(context);
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
        for (std::size_t i = 0; i < live_particle_count_;)
        {
            auto delta_origin = update.new_origin - emission_origin_;

            auto &particle = particles_[i];

            particle.age += update.delta_time;
            if (particle.age >= particle.time_to_live)
            {
                kill_(i);
                continue;
            }

            auto context = ParticleUpdateContext{
                .particle   = particle,
                .delta_time = update.delta_time,
            };

            particle.acceleration = scheme_.acceleration(context);
            particle.velocity += particle.acceleration * update.delta_time;
            particle.position += particle.velocity * update.delta_time;
            particle.position += delta_origin * scheme_.emitter_pull;
            particle.angle += particle.angular_speed * update.delta_time;
            particle.color = scheme_.color(context);
            particle.scale = scheme_.scale(context);

            ++i;
        }

        emission_origin_ = update.new_origin;
    }

    void
    render(ome::Vec3f camera_up, ome::Vec3f camera_right) const
    {
        glEnable(GL_BLEND);
        glBlendFunc(scheme_.blend_mode.source_factor, scheme_.blend_mode.destination_factor);
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

}
