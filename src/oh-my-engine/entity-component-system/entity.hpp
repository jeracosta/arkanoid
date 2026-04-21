#pragma once

#include <cstddef>
#include <format>

#include "oh-my-engine/entity-component-system/component.hpp"

namespace ome::ecs {

class EntityStore; // forward declaration

class Entity
{
  public:
    struct Identifier
    {
        std::size_t   index;
        unsigned char generation;

        bool
        operator<=>(const Identifier &other) const
            = default;

        operator std::size_t() const
        {
            return index;
        }
    };

  private:
    Identifier   id_;
    EntityStore &store_;

    // NOTE: Entities exist as indices inside an entity store. Instances of this class act like
    // handles to the actual entities, and their validity depends on the store.

    Entity(const Identifier &id, EntityStore &store)
        : id_(id),
          store_(store)
    {
    }

    friend class EntityStore;

  public:
    Identifier
    id() const
    {
        return id_;
    }

    bool
    is_alive() const; // depends on the store

    void
    kill(); // depends on the store

    template <IsComponent Component>
    Component *
    get(); // depends on the store

    operator Identifier() const
    {
        return id_;
    };
};

} // namespace ome::ecs

#include <format>

template <>
struct std::formatter<ome::ecs::Entity::Identifier>
{
    constexpr auto
    parse(std::format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <class FormatContext>
    auto
    format(const ome::ecs::Entity::Identifier &id, FormatContext &ctx) const
    {
        return std::format_to(ctx.out(), "{{index={}, generation={}}}", id.index, id.generation);
    }
};
