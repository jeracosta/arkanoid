#pragma once

#include <array>
#include <boost/container/static_vector.hpp>
#include <vector>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/entity-component-system/component.hpp"
#include "oh-my-engine/entity-component-system/entity.hpp"

namespace ome::ecs {

template <IsComponent Component>
class ComponentStore
{
  private:
    using Index = std::size_t; // each component instance has an index

    static constexpr auto null_index = std::numeric_limits<Index>::max();
    static_assert(null_index > max_entities);

    std::vector<Component> components_; // storage of components

    std::vector<Index>              component_to_entity_; // index map
    std::array<Index, max_entities> entity_to_component_; // index map

  public:
    ComponentStore()
    {
        entity_to_component_.fill(null_index);
    }

    const Component *
    operator[](Entity::Identifier entity) const
    {
        auto index = entity_to_component_[entity];
        if (index == null_index)
        {
            return nullptr;
        }
        return &components_[index];
    }

    const Component *
    set(Entity::Identifier entity_id, Component component = {})
    {
        if (auto stored_ = (*this)[entity_id])
        {
            *stored_ = std::move(component);
            return stored_;
        }

        entity_to_component_[entity_id] = components_.size();
        component_to_entity_.push_back(entity_id);
        components_.push_back(std::move(component));
    }

    void
    erase(Entity::Identifier entity)
    {
        auto index = entity_to_component_[entity];

        if (index == null_index || components_.empty())
        {
            return;
        }

        std::swap(components_[index], components_.back());
        components_.pop_back();

        std::swap(component_to_entity_[index], component_to_entity_.back());
        component_to_entity_.pop_back();

        entity_to_component_[component_to_entity_[index]] = index;
        entity_to_component_[entity]                      = null_index;
    }
};

} // namespace ome::ecs
