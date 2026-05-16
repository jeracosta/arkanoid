#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <cxxabi.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

#include "oh-my-engine/game.hpp"
#include "oh-my-engine/input.hpp"
#include "soccernoid/constants.hpp"
#include "soccernoid/input.hpp"
#include "soccernoid/nodes/root.hpp"

using namespace ome;
using namespace soccernoid;

int
main()
{
    Game::run({
      .window = {
          .title = "Soccernoid",
          .size  = {640, 480},
          .resizeable = true,
      },

      .camera = {
          .distance = 15,
      },

      .fog = {
          .color   = colors.fog,
          .start   = fog.start,
          .end     = fog.end,
          .enabled = true,
      },

      .lighting = {},

      .make_input_mapper = [&](auto &)
      {
          using namespace input;
          using enum KeyInput;

          auto inputs = InputMapper();

          configure_default_controls(&inputs);

          return inputs;
      },

      .make_root_node = [&] (Game &)
      {
          return std::make_shared<RootNode>();
      },
    });

    return EXIT_SUCCESS;
}
