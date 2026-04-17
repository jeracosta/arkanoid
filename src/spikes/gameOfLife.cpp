#include "oh-my-engine/game.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <glm/vec2.hpp>
#include <print>
#include <vector>

enum class Action : uint8_t
{
    Quit,
    Pause,
    Resume,
    Clear,
    Randomize,
    IncreaseResolution,
    DecreaseResolution,
    IncreaseSpeed,
    DecreaseSpeed,
};

struct Cell
{
    bool alive = false;
};

class Board
{
  private:
    // Determines board size; cells per side.
    std::size_t resolution_;

    // Vectorized cell matrix. Includes phantom border of dead cells to optimize neighbor counting.
    std::vector<Cell> cells_;

    // Double buffer to store next state of the board while updating.
    std::vector<Cell> cells_buffer_;

    std::size_t
    index_(glm::ivec2 pos) const
    {
        return static_cast<std::size_t>(pos.y + 1) * (resolution_ + 2)
               + static_cast<std::size_t>(pos.x + 1);
    }

    template <typename Fn>
    void
    for_each_cell_(Fn &&fn)
    {
        for (std::size_t y = 0; y < resolution_; ++y)
        {
            for (std::size_t x = 0; x < resolution_; ++x)
            {
                fn(glm::ivec2{ static_cast<int>(x), static_cast<int>(y) });
            }
        }
    }

    template <typename Fn>
    void
    for_each_cell_(Fn &&fn) const
    {
        for (std::size_t y = 0; y < resolution_; ++y)
        {
            for (std::size_t x = 0; x < resolution_; ++x)
            {
                fn(glm::ivec2{ static_cast<int>(x), static_cast<int>(y) });
            }
        }
    }

    auto
    neighbors_(glm::ivec2 pos) const
    {
        static constexpr glm::ivec2 offsets[] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 },
                                                  { 1, 0 },   { -1, 1 }, { 0, 1 },  { 1, 1 } };

        return std::array<const Cell *, 8>{
            &cells_[index_(pos + offsets[0])], &cells_[index_(pos + offsets[1])],
            &cells_[index_(pos + offsets[2])], &cells_[index_(pos + offsets[3])],
            &cells_[index_(pos + offsets[4])], &cells_[index_(pos + offsets[5])],
            &cells_[index_(pos + offsets[6])], &cells_[index_(pos + offsets[7])]
        };
    }

  public:
    static constexpr auto default_resolution = 64;

    Board(std::size_t resolution)
        : resolution_(resolution),
          cells_((resolution + 2) * (resolution + 2)),
          cells_buffer_((resolution + 2) * (resolution + 2))
    {
    }

    Board()
        : Board(default_resolution)
    {
    }

    std::size_t
    resolution() const
    {
        return resolution_;
    }

    std::size_t
    row_count() const
    {
        return resolution_;
    }

    std::size_t
    column_count() const
    {
        return resolution_;
    }

    std::size_t
    cell_count() const
    {
        return resolution_ * resolution_;
    }

    bool
    inside(glm::uvec2 pos) const
    {
        return pos.x < resolution_ && pos.y < resolution_;
    }

    Cell &
    operator[](const glm::uvec2 &pos)
    {
        assert(inside(pos));
        return cells_[index_(glm::ivec2{ static_cast<int>(pos.x), static_cast<int>(pos.y) })];
    }

    const Cell &
    operator[](const glm::uvec2 &pos) const
    {
        assert(inside(pos));
        return cells_[index_(glm::ivec2{ static_cast<int>(pos.x), static_cast<int>(pos.y) })];
    }

    auto
    begin()
    {
        return cells_.begin();
    }

    auto
    end()
    {
        return cells_.end();
    }

    auto
    begin() const
    {
        return cells_.begin();
    }

    auto
    end() const
    {
        return cells_.end();
    }

    void
    update()
    {
        std::fill(cells_buffer_.begin(), cells_buffer_.end(), Cell{});

        for_each_cell_([this](glm::ivec2 pos)
        {
            auto &cell      = cells_[index_(pos)];
            auto &next_cell = cells_buffer_[index_(pos)];

            int alive_neighbors = 0;
            for (const Cell *neighbor : neighbors_(pos))
            {
                alive_neighbors += neighbor->alive ? 1 : 0;
            }

            if (cell.alive)
            {
                next_cell.alive = alive_neighbors == 2 || alive_neighbors == 3;
            }
            else
            {
                next_cell.alive = alive_neighbors == 3;
            }
        });

        std::swap(cells_, cells_buffer_);
    }
};

class GameOfLife
{
  private:
    Board board_{};
    float speed_       = default_speed;
    bool  paused_      = false;
    float accumulator_ = 0.0f;

  public:
    static constexpr auto default_speed = 1.0f;
    static constexpr auto speed_factor  = 1.1f;
    static constexpr auto min_speed     = 0.1f;
    static constexpr auto max_speed     = 100.0f;
    static constexpr auto random_chance = 0.5f;

    void
    toggle_pause()
    {
        paused_ = !paused_;
        std::println("{}", paused_ ? "Paused" : "Resumed");
    }

    bool
    is_paused() const
    {
        return paused_;
    }

    void
    increase_speed()
    {
        speed_ = std::min(speed_ * speed_factor, max_speed);
        std::println("Speed: {:.2f} steps/s", speed_);
    }

    void
    decrease_speed()
    {
        speed_ = std::max(speed_ / speed_factor, min_speed);
        std::println("Speed: {:.2f} steps/s", speed_);
    }

    void
    increase_resolution()
    {
        board_ = Board{ board_.resolution() + 1 };
        std::println("Resolution: {}x{}", board_.resolution(), board_.resolution());
    }

    void
    decrease_resolution()
    {
        if (board_.resolution() > 1)
        {
            board_ = Board{ board_.resolution() - 1 };
        }
        std::println("Resolution: {}x{}", board_.resolution(), board_.resolution());
    }

    void
    clear()
    {
        board_ = Board{ board_.resolution() };
        std::println("Cleared");
    }

    void
    randomize()
    {
        for (std::size_t y = 0; y < board_.row_count(); ++y)
        {
            for (std::size_t x = 0; x < board_.column_count(); ++x)
            {
                board_[glm::uvec2{ static_cast<unsigned>(x), static_cast<unsigned>(y) }].alive
                    = (std::rand() / static_cast<float>(RAND_MAX)) < random_chance;
            }
        }
        std::println("Randomized");
    }

    void
    tick(float delta_time)
    {
        if (paused_)
        {
            return;
        }

        accumulator_ += delta_time;

        const float step = 1.0f / speed_;
        while (accumulator_ >= step)
        {
            board_.update();
            accumulator_ -= step;
        }
    }

    friend void
    render(const GameOfLife &game)
    {
        const auto &board = game.board_;

        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        for (std::size_t y = 0; y < board.row_count(); ++y)
        {
            for (std::size_t x = 0; x < board.column_count(); ++x)
            {
                const glm::uvec2 pos{ static_cast<unsigned>(x), static_cast<unsigned>(y) };
                if (!board[pos].alive)
                {
                    continue;
                }

                const float fx0
                    = -1.0f + 2.0f * static_cast<float>(x) / static_cast<float>(board.resolution());
                const float fy0
                    = -1.0f + 2.0f * static_cast<float>(y) / static_cast<float>(board.resolution());
                const float fx1
                    = -1.0f
                      + 2.0f * static_cast<float>(x + 1) / static_cast<float>(board.resolution());
                const float fy1
                    = -1.0f
                      + 2.0f * static_cast<float>(y + 1) / static_cast<float>(board.resolution());

                const float pad_x = (fx1 - fx0) * 0.08f;
                const float pad_y = (fy1 - fy0) * 0.08f;

                glVertex2f(fx0 + pad_x, fy0 + pad_y);
                glVertex2f(fx1 - pad_x, fy0 + pad_y);
                glVertex2f(fx1 - pad_x, fy1 - pad_y);
                glVertex2f(fx0 + pad_x, fy1 - pad_y);
            }
        }
        glEnd();
    }
};

int
main()
{
    auto gol = GameOfLife{};

    ome::game::run({
        .window = {
            .title = "Game of life",
            .size  = { 640, 640 },
        },

        .configure_input = [&](auto &inputs, auto &game)
        {
            using enum ome::input::KeyInput;
            inputs.keyboard.bind(SDLK_ESCAPE,   Release,         [&]{ game.stop(); });
            inputs.keyboard.bind(SDLK_SPACE,    Press,           &GameOfLife::toggle_pause,        &gol);
            inputs.keyboard.bind(SDLK_c,        Press,           &GameOfLife::clear,               &gol);
            inputs.keyboard.bind(SDLK_r,      { Press, Repeat }, &GameOfLife::randomize,           &gol);
            inputs.keyboard.bind(SDLK_UP,     { Press, Repeat }, &GameOfLife::increase_speed,      &gol);
            inputs.keyboard.bind(SDLK_DOWN,   { Press, Repeat }, &GameOfLife::decrease_speed,      &gol);
            inputs.keyboard.bind(SDLK_RIGHT,  { Press, Repeat }, &GameOfLife::increase_resolution, &gol);
            inputs.keyboard.bind(SDLK_LEFT,   { Press, Repeat }, &GameOfLife::decrease_resolution, &gol);
        },

        .on_update = [&gol](const auto &context)
        {
            gol.tick(context.time.delta());
            render(gol);
        }
    });

    return EXIT_SUCCESS;
}
