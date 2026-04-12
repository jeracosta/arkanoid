#include "application.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <glm/gtc/noise.hpp>

float
noise_value(float angle, float time)
{
    auto direction  = glm::vec2{ cos(angle), sin(angle) };
    auto raw        = glm::perlin(glm::vec3(direction, time));
    auto normalized = raw * 0.5f + 0.5f; // Map from [-1, 1] to [0, 1]
    return normalized;
}

glm::vec4
noise_color(float time)
{
    return {
        noise_value(0.0f / 2, time),
        noise_value(1.0f / 2, time),
        noise_value(2.0f / 2, time),
        1.0f,
    };
}

class FlashColor
{
  private:
    float           intensity_  = 0;
    const float     decay_rate_ = 1.0 / 10;
    const glm::vec4 color_      = glm::vec4(1.0f);

  public:
    void
    trigger()
    {
        intensity_ = 1.0f;
    }

    void
    update(float delta_time)
    {
        intensity_ *= std::pow(decay_rate_, delta_time);
    }

    friend glm::vec4
    mix(const glm::vec4 &color, const FlashColor &flash)
    {
        return glm::mix(color, flash.color_, flash.intensity_);
    }
} flash;

int
main()
{
    auto app = Application({
      .window = {
        .title = "Test SDL app",
        .size  = {640, 480},
      },

      .input_setup = [](auto &inputs, auto &context)
      {
          inputs.bind(SDLK_ESCAPE, KeyInput::Release, InputAction{context.stop});
          inputs.bind(SDLK_SPACE,  KeyInput::Press,   []{ flash.trigger(); }   );
      },

      .init = []{},

      .frame_logic = [](const Application::RuntimeContext &frame)
      {
          auto color = mix(noise_color(frame.time.elapsed), flash);
          flash.update(frame.time.delta);

          glClearColor(color.r, color.g, color.b, color.a);
          glClear(GL_COLOR_BUFFER_BIT);
      }
    });

    app.run();

    return EXIT_SUCCESS;
}
