#pragma once

#include "node.hpp"

namespace ome {

class NodeCompositionCursor
{
    std::vector<Node *> stack_;

  public:
    template <class Root>
        requires std::derived_from<std::remove_cvref_t<Root>, Node>
    explicit NodeCompositionCursor(Root &root)
    {
        stack_.push_back(&root);
    }

    NodeCompositionCursor &
    add(std::shared_ptr<Node> child)
    {
        auto *parent = stack_.back();
        auto *node   = parent->add_child(std::move(child));
        stack_.push_back(node);
        return *this;
    }

    template <class T, class... Args>
        requires std::derived_from<T, Node>
    NodeCompositionCursor &
    add(Args &&...args)
    {
        return add(std::make_shared<T>(std::forward<Args>(args)...));
    }

    NodeCompositionCursor &
    named(std::string name)
    {
        stack_.back()->rename(std::move(name));

        return *this;
    }

    NodeCompositionCursor &
    up()
    {
        if (stack_.size() == 1)
        {
            throw std::runtime_error("Node builder tried to move up from root node.");
        }

        stack_.pop_back();

        return *this;
    }

    operator Node &()
    {
        return *stack_.front(); // root
    }
};

[[nodiscard]] inline NodeCompositionCursor
extending(Node &root)
{
    return NodeCompositionCursor(root);
}

} // namespace ome
