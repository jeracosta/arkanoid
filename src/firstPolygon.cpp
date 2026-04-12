#include "application.hpp"
#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <GL/glu.h>

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
      },

      .init = []
      {
          glClearColor(0.0,0.0,0.0,1.0); //black
          glMatrixMode(GL_PROJECTION);
          //gluPerspective(45,(640.0/480.0),0.1,100);
      },

      .frame_logic = [](const Application::RuntimeContext &)
      {
          glClear(GL_COLOR_BUFFER_BIT);

      }
    });

    app.run();

    return EXIT_SUCCESS;
}
