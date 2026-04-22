#pragma once

#include <array>
#include <bitset>
#include <boost/container/static_vector.hpp>
#include <boost/mp11.hpp>
#include <format>
#include <stack>

#include "oh-my-engine/constants.hpp"
#include "oh-my-engine/entity-component-system/component.hpp"
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

    using ComponentStores_     = boost::mp11::mp_transform<ComponentStore, Components>;
    using ComponentStoreTuple_ = boost::mp11::mp_apply<std::tuple, ComponentStores_>;

    ComponentStoreTuple_ component_stores_;

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
    living_count() const
    {
        return living_entity_count_;
    }

    bool
    full() const
    {
        return living_entity_count_ >= max_entities;
    }

    Entity
    random(auto &rng)
    {
        auto index = std::uniform_int_distribution<Index>{ 0, next_new_index_ - 1 }(rng);
        return entity_handle_(index);
    }

    template <class... Components>
    Entity
    emplace(Components &&...components)
    {
        auto index = free_index_();

        assert(!occupied_indices_.test(index));
        occupied_indices_.set(index);

        ++living_entity_count_;
        // the generation gets incremented on kill

        auto entity = entity_handle_(index);

        (std::get<ComponentStore<std::decay_t<Components>>>(component_stores_)
             .set(entity.id(), std::forward<Components>(components)),
         ...);

        return entity;
    }

    template <class... Components>
    void
    emplace_many(std::size_t ammount, Components &&...components)
    {
        for (auto _ : std::views::iota(0u, ammount))
        {
            emplace(components...);
        }
    }

    bool
    is_alive(const Entity::Identifier &id) const
    {
        auto index = id.index;

        if (index >= next_new_index_ || index >= max_entities)
        {
            return false;
        }

        return occupied_indices_.test(index) && generation_[index] == id.generation;
    }

    Entity
    operator[](this auto &&self, const Entity::Identifier &id)
    {
        return { id, self };
    }

    auto
    living(this auto &&self)
    {
        using namespace std::views;

        auto dirty_indices  = iota(0u, self.next_new_index_);
        auto occupied       = [&](auto index) { return self.occupied_indices_.test(index); };
        auto to_entity_view = [&](auto index) { return self.entity_handle_(index); };

        return dirty_indices | filter(occupied) | transform(to_entity_view);
    }

    template <class... Components>
        requires(sizeof...(Components) > 0)
    auto
    living(this auto &&self)
    {
        auto filter = [&](auto entity) { return (entity.template get<Components>() && ...); };
        return self.living_entities() | std::views::filter(filter);
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

    void
    kill_all()
    {
        for (auto entity : living())
        {
            entity.kill();
        }
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

template <class Component>
inline Component *
Entity::get()
{
    using ComponentStore = ComponentStore<std::decay_t<Component>>;
    auto &store          = std::get<ComponentStore>(store_.component_stores_);
    return store[id_];
}

} // namespace ome::ecs
