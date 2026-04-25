#pragma once

#include <concepts>
#include <functional>

#include "oh-my-engine/node.hpp"

namespace ome {

template <class TNode>
    requires std::derived_from<TNode, Node>
class Scripted : public TNode
{
  public:
    struct Configuration
    {
        std::function<void(Node &)> on_mount   = {};
        std::function<void(Node &)> on_ready   = {};
        std::function<void(Node &)> on_tick    = {};
        std::function<void(Node &)> on_unmount = {};
    } callbacks;

    template <class... Args>
    Scripted(Args &&...args, Configuration config = {})
        : TNode(std::forward<Args>(args)...),
          callbacks(std::move(config))
    {
    }

    void
    on_mount_() override
    {
        if (callbacks.on_mount)
        {
            callbacks.on_mount();
        }

        TNode::on_mount_();
    }

    void
    on_ready_() override
    {
        if (callbacks.on_ready)
        {
            callbacks.on_ready();
        }

        TNode::on_ready_();
    }

    void
    tick_() override
    {
        if (callbacks.on_tick)
        {
            callbacks.on_tick();
        }

        TNode::tick_();
    }

    void
    on_unmount_() override
    {
        if (callbacks.on_unmount)
        {
            callbacks.on_unmount();
        }

        TNode::on_unmount_();
    }
};

} // namespace ome
