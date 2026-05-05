#include "oh-my-engine/node.hpp"

namespace soccernoid {

class TerrainNode : public ome::Node
{
  private:
    void
    render_cancha_()
    {
        glBegin(GL_QUADS);
        {
            glColor(ome::Color::rgb(0.1, 0.8, 0.1));
            glVertex3f(-1.0, 0.0, 1.0);
            glVertex3f(1.0, 0.0, 1.0);
            glVertex3f(1.0, 0.0, -1.0);
            glVertex3f(-1.0, 0.0, -1.0);
        }
        glEnd();
    }

    void
    render_arco_()
    {
        glBegin(GL_QUADS);
        {
            glColor(ome::Color::rgb(0.9, 0.95, 0.85));
            glVertex3f(.25, 0.0, -1.0);
            glVertex3f(.25, 0.2, -1.0);
            glVertex3f(-.25, 0.2, -1.0);
            glVertex3f(-.25, 0.0, -1.0);
        }
        glEnd();
    }

  public:
    void
    on_tick_() override
    {
        render_cancha_();
        render_arco_();
    }
};

} // namespace soccernoid
