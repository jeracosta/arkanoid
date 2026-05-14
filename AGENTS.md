# AGENTS.md

## Project map

- `src/oh-my-engine/` = reusable engine library (`ome`)
- `src/soccernoid/main.cpp` = entrypoint for game executable
- `src/soccernoid/nodes/root.hpp` = top-level gameplay wiring
- `src/spikes/` = experiments, not part of `game`

## Build / run

- Build: `./scripts/build.sh game`
- Run: `./build/soccernoid/bin/game`
- If changing build type or CMake options, reconfigure manually with `cmake -B build ...`

## Tooling

- Use `.clang-format`
- No checked-in tests; verify with a focused build of the touched target

## Important caveats

- Only target defined for the app is `game`
- `scripts/build-mingw64.sh` and editor tasks may be stale; prefer CMake commands above
