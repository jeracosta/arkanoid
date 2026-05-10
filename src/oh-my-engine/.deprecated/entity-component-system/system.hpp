#include "oh-my-engine/entity-component-system/entity.hpp"

namespace ome {

class Game;

namespace ecs {

class System
{
  public:
    virtual void
    update(Entity &entity, Game &game)
        = 0;

    virtual ~System() = default;
};
} // namespace ecs
} // namespace ome
