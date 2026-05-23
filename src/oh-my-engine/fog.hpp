#include "oh-my-engine/color.hpp"
namespace ome {

struct Fog
{
    Color color;
    float start_distance = 1.0f;   // GL_LINEAR only
    float end_distance   = 100.0f; // GL_LINEAR only
    float density        = 0.05f;  // GL_EXP / GL_EXP2

    enum class Mode
    {
        Linear = GL_LINEAR,
        Exp    = GL_EXP,
        Exp2   = GL_EXP2
    } mode
        = Mode::Linear;

    enum class Hint
    {
        Fast     = GL_FASTEST,
        Nicest   = GL_NICEST,
        DontCare = GL_DONT_CARE
    } hint
        = Hint::Nicest;
};

} // namespace ome
