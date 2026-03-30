#include "application.hpp"
#include "ray_march.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>

using namespace glm;

struct Pixel
{
    uint8_t r, g, b;
};

class PixelBuffer
{
    std::vector<Pixel> data_;
    glm::uvec2         size_;

  public:
    PixelBuffer(glm::uvec2 size)
        : data_(size.x * size.y),
          size_(size)
    {
    }

    Pixel &
    operator[](glm::uvec2 coord)
    {
        return data_[coord.y * size_.x + coord.x];
    }

    uint8_t *
    data()
    {
        return reinterpret_cast<uint8_t *>(data_.data());
    }

    auto
    coords_range() const
    {
        using namespace std::ranges;

        auto x_range = views::iota(0u, size_.x);
        auto y_range = views::iota(0u, size_.y);

        auto as_uvec2 = [](auto pair)
        {
            auto [x, y] = pair;
            return glm::uvec2(x, y);
        };

        return views::cartesian_product(x_range, y_range) | views::transform(as_uvec2);
    }
};

struct
{
    GLuint     descriptor;
    glm::uvec2 size;
} texture;

struct Camera
{
    vec3  position;
    vec3  target;
    vec3  up;
    float fov;

    struct Basis
    {
        vec3 forward, right, up;
    };

    Basis
    basis() const
    {
        vec3 forward = normalize(target - position);
        vec3 right   = normalize(cross(forward, up));
        vec3 up      = cross(right, forward);
        return { .forward = forward, .right = right, .up = up };
    }
};

std::chrono::seconds second;

void
frame(const Application::RuntimeContext &context)
{

    float time = context.time.elapsed * 0.5;

    auto camera = Camera{
        .position = { 2.5 * sin(time), 0, 2.5 * cos(time) },
        .target   = { 0, 0, 0 },
        .up       = { 0, 1, 0 },
        .fov      = 45.0,
    };

    auto basis = camera.basis();

    float aspect = float(texture.size.x) / float(texture.size.y);
    float scale  = tan(radians(camera.fov * 0.5));

    PixelBuffer pixels(texture.size);

    auto fractal = MengerSponge{};
    auto pyramid = Pyramid{ 1, 1 };

    auto sdf = [&](vec3 p) { return pyramid.distance(p); };

    for (auto coords : pixels.coords_range())
    {
        // Normalize coords to [-1,1] screen space
        vec2 uv = (vec2(coords) / vec2(texture.size)) * 2.0f - 1.0f;

        // Ray direction in camera space
        vec3 ray_direction = normalize(basis.forward + basis.right * uv.x * aspect * scale
                                       + basis.up * uv.y * scale);

        float epsilon = 0.025f;

        auto ray_march_result
            = ray_march(camera.position, ray_direction, sdf, 200, 100.0f, epsilon);

        vec3 color(0.0f); // background color
        if (ray_march_result.hit)
        {
            color      = vec3(1.0f, 0.7f, 0.3f) * 28.0f;
            float glow = std::exp(-ray_march_result.closest_distance * 1.2f);
            color *= glow;
        }
        else
        {
            float glow = std::exp(-ray_march_result.closest_distance * 5.2f) * 1.2f;
            color      = vec3(1.0f, 0.7f, 0.3f) * glow;
        }

        // Convert to 8-bit color
        auto to8bit
            = [](float v) { return static_cast<uint8_t>(std::clamp(v * 255.0f, 0.0f, 255.0f)); };
        pixels[coords] = { to8bit(color.r), to8bit(color.g), to8bit(color.b) };
    }

    glBindTexture(GL_TEXTURE_2D, texture.descriptor);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 texture.size.x,
                 texture.size.y,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 pixels.data());

    // Draw fullscreen quad
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, -1);
    glTexCoord2f(1, 0);
    glVertex2f(1, -1);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(-1, 1);
    glEnd();

    {
        using std::chrono::duration_cast;
        using std::chrono::seconds;
        using TimeUnit = Application::RuntimeContext::TimeUnit;

        auto previous_second = second;
        second               = duration_cast<seconds>(TimeUnit(context.time.elapsed));

        if (previous_second != second)
        {
            auto fps = 1.0f / context.time.delta;
            std::printf("\tFPS: %.3f\r", fps);
            std::fflush(stdout);
        }
    }
}

int
main()
{
    texture.size = { 512, 512 };

    auto app = Application({
      .window = {
        .title = "Test SDL app",
        .size  = texture.size,
      },

      .input_setup = [](auto &inputs, auto &context)
      {
          inputs.bind(SDLK_ESCAPE, KeyInput::Release, InputAction{context.stop});
      },

      .frame_logic = frame
    });

    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture.descriptor);
    glBindTexture(GL_TEXTURE_2D, texture.descriptor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 texture.size.x,
                 texture.size.y,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 nullptr);

    app.run();

    return EXIT_SUCCESS;
}
