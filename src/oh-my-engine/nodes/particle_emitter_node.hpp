#pragma once

#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/result_type.hpp>
#include <functional>
#include <limits>
#include <optional>
#include <random>

#include "oh-my-engine/interpolation.hpp"
#include "oh-my-engine/math/vector.hpp"
#include "oh-my-engine/nodes/transform_node.hpp"
#include "oh-my-engine/particles_server.hpp"

namespace ome {

class ParticleEmitterNode : public TransformNode
{
  public:
    struct Settings
    {
        ParticleScheme particle_blueprint;
        float          trigger_rate;              // how many times it triggers per time unit
        uint           particles_per_trigger = 1; // how many particles to emit at each trigger
        bool           use_unscaled_time     = false;
    };

    ParticleEmitterNode(Settings settings)
        : particles_(settings.particle_blueprint, transform<Space::World>().position),
          settings_(std::move(settings))
    {
    }

    void
    on_tick_() override
    {
        const float delta_time
            = settings_.use_unscaled_time ? game()->time.unscaled.delta() : game()->time.delta();

        const auto origin = transform<Space::World>().position;

        const float period = settings_.trigger_rate > 0.0f ? 1.0f / settings_.trigger_rate
                                                           : std::numeric_limits<float>::infinity();

        if (!origin_)
        {
            origin_ = origin;
        }

        [[unlikely]]
        if (!std::isfinite(period))
        {
            particles_.update({ delta_time, origin });
        }
        else
        {
            const float time_since_last_trigger = accumulator_;
            float       cursor                  = 0.0f;
            float       next_trigger_at         = period - time_since_last_trigger;

            while (next_trigger_at <= delta_time)
            {
                const float segment  = next_trigger_at - cursor;
                const float progress = delta_time > 0.0f ? next_trigger_at / delta_time : 1.0f;

                const Vec3f interpolated_origin = lerp(*origin_, origin, progress);

                particles_.update({ segment, interpolated_origin });
                particles_.emit(settings_.particles_per_trigger);

                cursor = next_trigger_at;
                next_trigger_at += period;
            }

            if (cursor < delta_time)
            {
                particles_.update({ delta_time - cursor, origin });
            }

            accumulator_ = std::fmod(time_since_last_trigger + delta_time, period);
        }

        origin_ = origin;

        // Rendering is deferred to the end-of-frame task drain so particles
        // compose on top of all opaque geometry. The schedule must not capture
        // a raw `this`: the emitter can be unmounted and destroyed earlier in
        // the same drain (e.g. when its parent despawns this frame). Using a
        // weak reference lets the task safely no-op in that case.
        auto self_weak = this->weak_from_this();
        auto &camera   = Node::game()->camera;
        game()->schedule([self_weak, &camera]
        {
            if (auto self = self_weak.lock())
            {
                static_cast<ParticleEmitterNode *>(self.get())->particles_.render(camera);
            }
        });
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
    std::optional<Vec3f> origin_;
    float                accumulator_ = 0.0f;

  protected:
    static inline std::mt19937 rng = {};
};

} // namespace ome
