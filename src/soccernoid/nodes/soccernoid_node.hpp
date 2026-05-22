#pragma once

#include <utility>

#include "oh-my-engine/node.hpp"
#include "soccernoid/soccernoid.hpp"

namespace soccernoid {

template <class TBase = ome::Node>
class SoccernoidNode : public TBase
{
  public:
    template <class... Args>
    explicit SoccernoidNode(Args &&...args)
        : TBase(std::forward<Args>(args)...)
    {
    }

    Soccernoid *
    game()
    {
        return static_cast<Soccernoid *>(TBase::game());
    }

    const Soccernoid *
    game() const
    {
        return static_cast<const Soccernoid *>(TBase::game());
    }
};

} // namespace soccernoid
