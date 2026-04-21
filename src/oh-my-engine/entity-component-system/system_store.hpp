#pragma once
#include <memory>
#include <vector>

#include "oh-my-engine/entity-component-system/system.hpp"

namespace ome::ecs {

class SystemStore
{
  private:
    std::vector<std::unique_ptr<System>> systems_;

  public:
    SystemStore()                             = default;
    explicit SystemStore(const SystemStore &) = default;

    template <class... Systems>
    SystemStore(Systems &&...systems)
    {
        (systems_.emplace_back(
             std::make_unique<std::decay_t<Systems>>(std::forward<Systems>(systems))),
         ...);
    }

    template <class SystemType, class... Args>
    SystemType &
    emplace(Args &&...args)
    {
        systems_.emplace_back({ std::forward<Args>(args)... });
    }

    void
    update(Entity &entity)
    {
        for (auto &system : systems_)
        {
            system->update(entity);
        }
    }

    auto &
    raw()
    {
        return systems_;
    }
};

} // namespace ome::ecs
