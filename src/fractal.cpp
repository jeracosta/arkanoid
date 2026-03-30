#include "application.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdlib>
#include <functional>

using namespace glm;

class Box
{
  private:
    vec3 half_extent_;

  public:
    Box(float side_length)
        : half_extent_(side_length / 2)
    {
    }

    float
    distance(vec3 p) const
    {
        vec3 q = abs(p) - vec3(half_extent_);
        return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
    }
};

// Consider the space as a tesselation of cubes of `cell_size` side length, this function maps any
// point `p` to the corresponding point in the cube centered at the origin.
vec3
repeat_centered(vec3 p, float cell_size)
{
    return mod(p + cell_size / 2, cell_size) - cell_size / 2;
}

struct MengerSponge
{
    float tile_period     = 2.0f;        // space repetition period
    int   levels          = 4;           // fractal iterations
    float iteration_scale = 2.7f;        // scale applied each fold
    vec3  fold_offset     = vec3(-1.0f); // translation after scaling

    float
    distance(vec3 p) const
    {
        p = repeat_centered(p, tile_period);

        float de_scale = 1.0f;

        for (int i = 0; i < levels; ++i)
        {
            p = abs(p);

            if (p.x < p.y)
                std::swap(p.x, p.y);
            if (p.x < p.z)
                std::swap(p.x, p.z);
            if (p.y < p.z)
                std::swap(p.y, p.z);

            p = p * iteration_scale + fold_offset;

            if (p.z > 1.0f)
                p.z -= 2.0f;

            de_scale *= iteration_scale;
        }

        return Box(1.0f).distance(p) / de_scale;
    }
};

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

float
ray_march(vec3   origin,
          vec3   direction,
          auto &&distance_field, // simil `float(*)(vec3)`
          int    max_step_count,
          float  max_step_size,
          float  max_ray_length,
          float  epsilon)
{
    float ray_length = 0.0;
    for (int i = 0; i < max_step_count; i++)
    {
        vec3  ray      = origin + direction * ray_length;
        float distance = distance_field(ray);
        if (distance < epsilon)
            return ray_length;
        ray_length += std::min(distance, max_step_size);
        if (ray_length > max_ray_length)
            break;
    }
    return std::numeric_limits<float>::infinity();
}

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

void
frame(const Application::RuntimeContext &context)
{

    float time = context.time.elapsed * 0.5;

    auto camera = Camera{
        .position = { 2.5 * sin(time), 1.0, 2.5 * cos(time) },
        .target   = { 0, 0, 0 },
        .up       = { 0, 1, 0 },
        .fov      = 45.0,
    };

    auto basis = camera.basis();

    float aspect = float(texture.size.x) / float(texture.size.y);
    float scale  = tan(radians(camera.fov * 0.5));

    PixelBuffer pixels(texture.size);

    auto fractal = MengerSponge{};

    auto sdf = [&](vec3 p) { return fractal.distance(p); };

    for (auto coords : pixels.coords_range())
    {
        // Normalize coords to [-1,1] screen space
        vec2 uv = (vec2(coords) / vec2(texture.size)) * 2.0f - 1.0f;
        uv.y *= -1.0f; // (camera space increases upwards, screen space increases downwards)

        // Ray direction in camera space
        vec3 ray_direction = normalize(basis.forward + basis.right * uv.x * aspect * scale
                                       + basis.up * uv.y * scale);

        float distance
            = ray_march(camera.position, ray_direction, sdf, 200, 100.0f, 100.0f, 0.0015f);

        vec3 color(0.0f); // background color
        if (distance != std::numeric_limits<float>::infinity())
        {
            vec3 point = camera.position + ray_direction * distance;

            color = vec3(1.0f, 0.7f, 0.3f) * std::exp(-distance * 0.6f) * 3.0f;
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

    std::printf("Frame time: %.3f seconds\n", context.time.elapsed);
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
