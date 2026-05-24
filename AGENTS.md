# AGENTS.md

## Project Map

- `CMakeLists.txt` defines two local targets: static library `ome` from `src/oh-my-engine/*.cpp` and executable `game` from `src/soccernoid/*.cpp`.
- `src/oh-my-engine/` is the reusable engine namespace `ome`; `src/soccernoid/` is the game-specific app and settings.
- Game startup is `src/soccernoid/main.cpp` -> `Soccernoid::run`; top-level node wiring is `src/soccernoid/nodes/root.hpp`, and level contents/swaps are in `src/soccernoid/nodes/level.hpp`.
- Soccernoid nodes should usually derive through `SoccernoidNode<>` when they need the typed `Soccernoid *game()` accessor.
- `ome::Node` is non-movable; modify node trees in `on_mount_`, not `on_ready_` (`on_ready_` is marked as a SIGSEGV risk for tree changes).
- `ome::input::Action` values must stay 0-indexed and contiguous because `InputMapper` indexes action slots with the enum value.

## Build / Run

- Build the game with `./scripts/build.sh game`; the script configures `build/` as Release only when `build/` does not already exist.
- Run with `./build/soccernoid/bin/game`; assets are copied post-build to `./build/soccernoid/assets`, matching the executable-relative `../assets/` lookup.
- There are no CTest/add_test targets or CI workflows in this repo; verify changes with a focused build, normally `./scripts/build.sh game`.
- CMake requires C++26, uses `ccache` if available, and enables `-Wall -Wextra -Wpedantic -Wno-changes-meaning` for GCC/Clang.

## Dependencies / Assets

- `SDL2_image` is required through `pkg-config`; SDL2, glm, Boost.MP11, assimp, and ImGui can be fetched by CMake when not found locally.
- ImGui is built manually in `CMakeLists.txt` with SDL2 + OpenGL2 backends; keep that aligned with the fixed-function OpenGL path.
- Use `scripts/skybox-gen.sh <cross-image> [output-dir] [prefix]` to split skybox cross images; it requires ImageMagick's `magick`.
- `scripts/build-mingw64.sh` and `.vscode/tasks.json` still reference target `main`, but CMake defines `game`; check/fix those before relying on them.

## Style / Workflow

- Format C++ with the repo's `.clang-format` (GNU base, 4 spaces, 100 columns, sorted/regrouped includes).
- Commit messages should be Conventional Commits, minimalist, and one logical change per commit.
