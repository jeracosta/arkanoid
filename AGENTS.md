# AGENTS.md

## Repo shape
- This is a small CMake-based C++ project, not a package-managed monorepo. There is no README, CI workflow, or existing repo instruction file checked in.
- `src/oh-my-engine/` builds the reusable static library target `ome`.
- `src/soccernoid/` builds the actual executable target `game`.
- `src/soccernoid/main.cpp` is the real app entrypoint; it calls `ome::Game::run(...)` and mounts `soccernoid::RootNode`.
- `src/soccernoid/nodes/root.hpp` is the best high-level gameplay wiring file; start there before diving into individual nodes.
- `src/spikes/` contains experiments/examples and is not part of the `game` target.

## Build and run
- Preferred focused build: `./scripts/build.sh game`
- Run the built game with: `./build/soccernoid/bin/game`
- `scripts/build.sh` only runs CMake configure when `build/` does not exist, and it hardcodes `-DCMAKE_BUILD_TYPE=Release` on that first configure. If you change CMake options or want a Debug build, rerun CMake manually instead of trusting the script:
  - `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --target game -j`
- `CMAKE_RUNTIME_OUTPUT_DIRECTORY` is `build/soccernoid/bin`, and a post-build step copies `assets/` into `build/soccernoid/assets`.

## Dependencies and environment
- Build configuration requires system `pkg-config`, OpenGL, and `SDL2_image`. On this machine, `./scripts/build.sh game` failed during configure because `SDL2_image` was missing.
- `SDL2`, `glm`, `Boost::mp11`, and `assimp` are fetched with `FetchContent` if not already installed.
- Assimp is configured narrowly: tests/tools are off, exporters are off, and only the `3DS` importer is enabled.

## Verified quirks worth knowing
- The only executable target defined in `CMakeLists.txt` is `game`.
- `scripts/build-mingw64.sh` still tries to build target `main`, so do not trust it as-is for Windows cross-builds without fixing the target name.
- `.vscode/tasks.json` has one stale task too: `Build Soccernoid` calls `scripts/build.sh main`, while `Build and Run Soccernoid` correctly uses `game`.
- `CMakeLists.txt` sets `CMAKE_CXX_STANDARD 26`; editor/LSP diagnostics may be noisy if the toolchain or compile commands are not configured for that standard yet.

## Style and verification
- Formatting is defined in `.clang-format` (GNU-based, 100-column limit, sorted/regrouped includes, 4-space indent). Run `clang-format` rather than guessing style.
- No test suite or test runner config is checked in. For changes here, the primary verification step is a focused build of the affected target, usually `game`.
