#pragma once

#include <array>
#include <bitset>
#include <boost/container/static_vector.hpp>
#include <boost/mp11.hpp>
#include <format>
#include <stack>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/entity-component-system/component_store.hpp"
#include "oh-my-engine/entity-component-system/entity.hpp"

namespace ome::ecs {

class EntityStore
{
  private:
    using Index = decltype(Entity::Identifier::index);

    using IndexPoolStorage_ = boost::container::static_vector<Index, max_entities>;
    std::stack<Index, IndexPoolStorage_> freed_index_pool_;

    std::bitset<max_entities>               occupied_indices_;
    std::array<unsigned char, max_entities> generation_;

    std::size_t living_entity_count_ = 0;

    Index next_new_index_ = 0;

    using ComponentStores_ = boost::mp11::mp_apply<std::tuple, Components>;
    ComponentStores_ component_stores_;

    friend class Entity;

    Index
    free_index_()
    {
        if (living_entity_count_ >= max_entities)
        {
            throw std::runtime_error("Maximum entity count exceeded.");
        }

        if (!freed_index_pool_.empty())
        {
            auto index = freed_index_pool_.top();
            freed_index_pool_.pop();
            return index;
        }
        else
        {
            return next_new_index_++;
        }
    }

    Entity
    entity_handle_(this auto &&self, Index index)
    {
        return { { index, self.generation_[index] }, self };
    }

  public:
    EntityStore()                             = default;
    explicit EntityStore(const EntityStore &) = default;
    EntityStore(EntityStore &&)               = default;

    consteval std::size_t
    capacity() const
    {
        return max_entities;
    }

    std::size_t
    living_entity_count() const
    {
        return living_entity_count_;
    }

    template <IsComponent... Components>
    Entity
    emplace(Components &&...components)
    {
        auto index = free_index_();

        assert(!occupied_indices_.test(index));
        occupied_indices_.set(index);

        ++living_entity_count_;
        // the generation gets incremented on kill

        (std::get<ComponentStore<std::decay_t<Components>>>(component_stores_)
             .emplace(index, std::forward<Components>(components)),
         ...);

        return entity_handle_(index);
    }

    bool
    is_alive(const Entity::Identifier &id) const
    {
        auto index = id.index;

        if (index >= next_new_index_)
        {
            return false;
        }

        return occupied_indices_.test(index) && generation_[index] == id.generation;
    }

    void
    kill(const Entity::Identifier &id)
    {
        auto index = id.index;

        if (!is_alive(id))
        {
            throw std::runtime_error(std::format("Attempted to kill the non-alive entity {}", id));
        }

        occupied_indices_.reset(index);

        --living_entity_count_;
        ++generation_[index];

        freed_index_pool_.push(index);
    }

    Entity
    operator[](this auto &&self, const Entity::Identifier &id)
    {
        return { id, self };
    }

    auto
    living_entities(this auto &&self)
    {
        using namespace std::views;

        auto dirty_indices  = iota(0u, self.next_new_index_);
        auto occupied       = [&](auto index) { return self.occupied_indices_.test(index); };
        auto to_entity_view = [&](auto index) { return self.entity_handle_(index); };

        return dirty_indices | filter(occupied) | transform(to_entity_view);
    }
};

inline bool
Entity::is_alive() const
{
    return store_.is_alive(id_);
}

inline void
Entity::kill()
{
    store_.kill(id_);
}

template <IsComponent Component>
inline Component *
Entity::get()
{
    return std::get<ComponentStore<std::decay_t<Component>>>(store_.component_stores_).get(id_);
}

} // namespace ome::ecs
