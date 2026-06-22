#pragma once

#include <utility>

#include "arkanoid/arkanoid.hpp"
#include "oh-my-engine/node.hpp"

namespace arkanoid {

template <class TBase = ome::Node>
class ArkanoidNode : public TBase
{
  public:
    template <class... Args>
    explicit ArkanoidNode(Args &&...args)
        : TBase(std::forward<Args>(args)...)
    {
    }

    Arkanoid *
    game()
    {
        return static_cast<Arkanoid *>(TBase::game());
    }

    const Arkanoid *
    game() const
    {
        return static_cast<const Arkanoid *>(TBase::game());
    }
};

} // namespace arkanoid
