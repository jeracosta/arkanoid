#include "oh-my-engine/game.hpp"
#include "oh-my-engine/ray_march.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>

using namespace glm;

glm::vec3
hsv2rgb(float h)
{
    float r = glm::clamp(std::abs(h * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float g = glm::clamp(2.0f - std::abs(h * 6.0f - 2.0f), 0.0f, 1.0f);
    float b = glm::clamp(2.0f - std::abs(h * 6.0f - 4.0f), 0.0f, 1.0f);
    return glm::vec3(r, g, b);
}

glm::vec3
rainbow(float distance, float repeatFactor = 1.0f)
{
    float t = glm::fract(distance * repeatFactor); // repeat every 1/repeatFactor units
    return hsv2rgb(t);
}

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

float
smin(float a, float b, float k)
{
    float h = glm::clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
    return glm::mix(b, a, h) - k * h * (1.0f - h);
}

float
smin(std::initializer_list<float> values, float k)
{
    auto it = values.begin();
    if (it == values.end())
        return 0.0f;

    float result = *it++;
    for (; it != values.end(); ++it)
    {
        float a = result;
        float b = *it;
        float h = glm::clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
        result  = glm::mix(b, a, h) - k * h * (1.0f - h);
    }
    return result;
}

void
frame(const ome::game::Session &game)
{

    auto camera = Camera{
        .position = { 3 * std::cos(game.time.elapsed() * 0.5f),
                      0,
                      3 * std::sin(game.time.elapsed() * 0.5f), },
        .target   = { 0, 0, 0 },
        .up       = { 0, 1, 0 },
        .fov      = 45.0,
    };

    auto basis = camera.basis();

    float aspect = float(texture.size.x) / float(texture.size.y);
    float scale  = tan(radians(camera.fov * 0.5));

    PixelBuffer pixels(texture.size);

    using namespace ome::ray_march;

    auto cube    = Cube{ 0.5f };
    auto pyramid = Pyramid{ 1.0f, 1.0f };
    auto fractal = MengerSponge{ .levels = 2, .iteration_scale = 3.0f, .fold_offset = vec3(-1.0f) };

    auto t = std::sin(game.time.elapsed() * 0.5f) * 0.5 + 0.5;

    auto sdf = [&](vec3 p)
    {
        return smin(
            {
                Sphere{ 0.3 }.distance(p - vec3{ 0, 0.5, 0 }),
                Sphere{ 0.5 }.distance(p - vec3{ 0, -0.5, 0 }),
                Cube{ 0.6 }.distance(p - vec3{ 0.5, -0.2, 0 }),
                Pyramid{ 0.6, 0.4 }.distance(p - vec3{ -0.8, 0.8, 0 }),
            },
            t);
    };

    for (auto coords : pixels.coords_range())
    {
        // Normalize coords to [-1,1] screen space
        vec2 uv = (vec2(coords) / vec2(texture.size)) * 2.0f - 1.0f;

        // Ray direction in camera space
        vec3 ray_direction = normalize(basis.forward + basis.right * uv.x * aspect * scale
                                       + basis.up * uv.y * scale);

        float epsilon = 0.001f;

        auto ray_march_result
            = ray_march(camera.position, ray_direction, sdf, 200, 100.0f, epsilon);

        auto color = vec3{ 0 };

        if (ray_march_result.hit)
        {
            glm::vec3 p = ray_march_result.hit->point;
            glm::vec3 n = ray_march_result.hit->normal;

            // simple directional light
            glm::vec3 light_dir = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f));
            float     diff      = glm::clamp(glm::dot(n, light_dir), 0.0f, 1.0f);

            // ambient + diffuse
            glm::vec3 ambient = 0.1f * glm::vec3(1.0f);
            glm::vec3 diffuse = diff * glm::vec3(1.0f, 0.9f, 0.7f);

            // rim lighting
            glm::vec3 view_dir  = glm::normalize(camera.position - p);
            float     rim       = pow(1.0f - glm::dot(n, view_dir), 2.0f);
            glm::vec3 rim_color = rim * glm::vec3(0.8f, 0.9f, 1.0f);

            color = ambient + diffuse + rim_color;
            color = glm::clamp(color, 0.0f, 1.0f);
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

        auto previous_second = second;
        second               = duration_cast<seconds>(ome::game::Time::Unit(game.time.elapsed()));

        if (previous_second != second)
        {
            auto fps = 1.0f / game.time.delta();
            std::printf("\tFPS: %.3f\r", fps);
            std::fflush(stdout);
        }
    }
}

int
main()
{
    texture.size = { 512, 512 };

    ome::game::run({
      .window = {
        .title = "Test SDL app",
        .size  = texture.size,
      },

      .configure_input = [](auto &inputs, auto &game)
      {
          using enum ome::input::KeyInput;
          inputs.keyboard.bind(SDLK_ESCAPE, Release, [&]{ game.stop(); });
      },

      .on_init = []()
      {
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
      },
  
      .on_update = frame
    });

    return EXIT_SUCCESS;
}
