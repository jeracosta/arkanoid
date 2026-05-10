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
    SystemStore() = default;

    SystemStore(const SystemStore &) = delete;

    SystemStore &
    operator=(const SystemStore &)
        = delete;

    SystemStore(SystemStore &&) = default;

    SystemStore &
    operator=(SystemStore &&)
        = default;

    template <class... Systems>
        requires((std::derived_from<std::decay_t<Systems>, System>) && ...)
    explicit SystemStore(Systems &&...systems)
    {
        (systems_.emplace_back(
             std::make_unique<std::decay_t<Systems>>(std::forward<Systems>(systems))),
         ...);
    }

    template <class System, class... Args>
    System &
    emplace(Args &&...args)
    {
        return systems_.emplace_back({ std::forward<Args>(args)... });
    }

    template <class... Systems>
        requires((std::derived_from<std::decay_t<Systems>, System>) && ...)
    void
    push(Systems &&...systems)
    {
        (systems_.emplace_back(
             std::make_unique<std::decay_t<Systems>>(std::forward<Systems>(systems))),
         ...);
    }

    template <class System>
    System &
    push(System &&system)
    {
        systems_.emplace_back(std::make_unique<System>(std::forward<System>(system)));
        return static_cast<System &>(*systems_.back());
    }

    void
    update(Entity &entity, Game &game)
    {
        for (auto &system : systems_)
        {
            system->update(entity, game);
        }
    }

    auto &
    raw()
    {
        return systems_;
    }

    std::size_t
    size() const
    {
        return systems_.size();
    }
};

} // namespace ome::ecs
