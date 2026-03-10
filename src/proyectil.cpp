#include "Vector3.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <print>
#include <ranges>
#include <vector>

using Seconds = std::chrono::duration<float>;

std::vector<Vector3>
free_fall_frames(Vector3 initial_velocity, Seconds time_step, Vector3 initial_position = {0.0f, 0.0f, 0.0f},)
{
    Vector3 acceleration = {0.0f, 0.0f , -9.81f};
    float dt = time_step.count();

    Vector3 delta_velocity = acceleration * dt;

    std::vector<Vector3> positions;
    Vector3 velocity = initial_velocity;
    Vector3 position = initial_position;
  
    while(position[2] > 0.0f && velocity[2] <= 0.0f)
    {
      positions.push_back(position);

      position += velocity * dt;
      velocity += delta_velocity;

    }
    positions.push_back(Vector3(0.0f, 0.0f, 0.0f));

    return positions;

}

struct TestCase
{
    struct
    {
        Vector3 initial_position;
        Vector3 initial_velocity;
        Seconds time_step;
    } scenario;

    std::vector<Vector3> expected_result;
};

auto test_cases = std::to_array<TestCase>({
  // 1: already on ground -> single frame (t=0)
  {
    .scenario = {
      .initial_position = {0.0f, 0.0f, 0.0f},
      .initial_velocity = {0.0f, 0.0f, 0.0f},
      .time_step = Seconds{1.0f}
    },
    .expected_result = {
      {0.0f, 0.0f, 0.0f},
    }
  },
  // 2: drop from 10 m, dt = 1s -> frames at t=0,1,2 (stop when y <= 0)
  {
    .scenario = {
      .initial_position = {0.0f, 10.0f, 0.0f},
      .initial_velocity = {0.0f, 0.0f, 0.0f},
      .time_step = Seconds{1.0f}
    },
    .expected_result = {
      {0.0f, 10.000000f, 0.0f},
      {0.0f,  5.095000f, 0.0f},
      {0.0f, -9.620000f, 0.0f},
    }
  },
  // 3: upward toss from 1 m, v=5 m/s, dt = 0.5s
  {
    .scenario = {
      .initial_position = {0.0f, 1.0f, 0.0f},
      .initial_velocity = {0.0f, 5.0f, 0.0f},
      .time_step = Seconds{0.5f}
    },
    .expected_result = {
      {0.0f,  1.000000f, 0.0f},
      {0.0f,  2.273750f, 0.0f},
      {0.0f,  1.095000f, 0.0f},
      {0.0f, -2.536250f, 0.0f},
    }
  },
  // 4: small negative initial vertical velocity, dt = 0.5s
  {
    .scenario = {
      .initial_position = {0.0f, 2.0f, 0.0f},
      .initial_velocity = {0.0f, -1.0f, 0.0f},
      .time_step = Seconds{0.5f}
    },
    .expected_result = {
      {0.0f,  2.000000f, 0.0f},
      {0.0f,  0.273750f, 0.0f},
      {0.0f, -3.905000f, 0.0f},
    }
  },
});

int
main()
{
    for (auto [i, test_case] : std::views::enumerate(test_cases))
    {
        std::println("Running test case {}...", (i + 1));

        auto result = free_fall_frames(test_case.scenario.initial_position,
                                       test_case.scenario.initial_velocity,
                                       test_case.scenario.time_step);

        bool ok = result.size() == test_case.expected_result.size()
                  && std::ranges::equal(result, test_case.expected_result);

        if (ok)
        {
            std::println("OK");
        }
        else
        {
            std::println("Failure!");
            std::println("Expected:");
            for (const auto &frame : test_case.expected_result)
            {
                std::println("  {}", frame);
            }
            std::println("Got:");
            for (const auto &frame : result)
            {
                std::println("  {}", frame);
            }
        }

        std::println();
    }

    return EXIT_SUCCESS;
}
