#include <memory>

#include "oh-my-engine/entity-component-system/entity.hpp"
namespace ome::ecs {

class System
{
  public:
    virtual void
    update(Entity &entity)
        = 0;

    virtual ~System() = default;
};

} // namespace ome::ecs
