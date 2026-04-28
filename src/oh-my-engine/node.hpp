// A node is the basic building block of a game.
// Nodes are "mounted" to become part of a game and begin affecting it in some way.
// Users can derive from a node to implement their own game logic.
// Nodes also have instance-level equivalent hooks, called after the class-level virtual hooks.
// Games tick all mounted nodes every frame, traversing the tree in a depth-first manner.
// Nodes can "die": a dead node doesn't tick, and gets unmounted and freed by the game.
// Additional specialized systems (e.g. rendering) could interact with nodes as well.
// A node can be mounted in three ways:
//   1. A game assigns it as the root node.
//   2. Another mounted node adds it as a child.
//   3. Its parent is mounted, recursively mounting all descendants.
// Unmounting a node unmounts all of its children as well.
// When recursively mounting a tree (node), nodes became "ready" when no children are left to mount.
// Nodes call virtual lifecycle hooks on mount (up-down), ready (down-up), and unmount (down-up).

#pragma once

#include <algorithm>
#include <boost/callable_traits.hpp>
#include <boost/type_index.hpp>
#include <cassert>
#include <flat_map>
#include <functional>
#include <memory>
#include <print>
#include <queue>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include "game.hpp"

namespace ome {

class Node : public std::enable_shared_from_this<Node>, public EventConnectionHolder
{
  public:
    struct Hooks
    {
        std::function<void(Node &)> on_mount   = {};
        std::function<void(Node &)> on_ready   = {};
        std::function<void(Node &)> on_tick    = {};
        std::function<void(Node &)> on_unmount = {};
    };

    Node(std::string_view name = {})
        : name_(name)
    {
    }

    // non-movable to avoid dangling pointers in parent and children
    Node(Node &&) = delete;

    Node &
    operator=(Node &&)
        = delete;

    std::string_view
    name() const
    {
        return name_;
    }

    std::string
    default_name() const
    {
        auto type_str     = boost::typeindex::type_id_runtime(*this).pretty_name();
        auto instance_str = std::format("{:x}", reinterpret_cast<std::uintptr_t>(this));

        return type_str + "@" + instance_str;
    }

    Game *
    game()
    {
        return game_;
    }

    Node *
    parent()
    {
        return parent_;
    }

    void
    rename(std::string new_name)
    {
        if (new_name == name_)
        {
            return;
        }

        if (!parent_)
        {
            name_ = std::move(new_name);
            return;
        }

        if (parent_->children_.contains(new_name))
        {
            throw std::runtime_error("Tried renaming node to sibling name.");
        }

        auto it   = parent_->children_.find(name_);
        auto node = std::move(it->second);
        parent_->children_.erase(it);

        name_ = std::move(new_name);
        parent_->children_.emplace(name_, std::move(node));
    }

    Node *
    child(const std::string &name)
    {
        auto it = children_.find(name);
        return it != children_.end() ? it->second.get() : nullptr;
    }

    bool
    is_mounted() const
    {
        return game_ != nullptr;
    }

    Node *
    add_child(std::shared_ptr<Node> child_owner)
    {
        auto child = child_owner.get();

        [[unlikely]]
        if (child->is_mounted())
        {
            throw std::runtime_error(
                "Tried reassigning a mounted node's parent; unmount it first.");
        }

        [[unlikely]]
        if (children_.contains(child->name_))
        {
            throw std::runtime_error(
                "Tried adding a child node to a node with a pre-existing child with the same "
                "name; rename first.");
        }

        children_.emplace(child->name_, std::move(child_owner));

        child->parent_ = this;

        if (is_mounted())
        {
            child->mount_to_(game_);
        }

        return child;
    }

    std::shared_ptr<Node>
    remove_child(const std::string_view &name)
    {
        auto it = children_.find(name);

        if (it == children_.end())
        {
            return nullptr;
        }

        auto child = std::move(it->second);

        children_.erase(it);

        child->unmount_();

        child->parent_ = nullptr;

        return child;
    }

    auto
    children(this auto &&self)
    {
        using namespace std::views;

        return self.children_ | values | transform(&std::shared_ptr<Node>::get);
    }

    void
    tick()
    {
        assert(is_mounted() && "Tried ticking an unmounted node; mount it first.");
        assert(is_alive && "Tried ticking a dead node; free it instead.");

        tick_();
        hooks_.on_tick ? hooks_.on_tick(*this) : void();
    }

    const Hooks &
    hooks() const
    {
        return hooks_;
    }

#define DEFINE_HOOK_SETTER(name)                                                                   \
    template <typename Callback>                                                                   \
    void hook_##name(Callback &&on_##name)                                                         \
    {                                                                                              \
        using CallbackArgs = boost::callable_traits::args_t<Callback>;                             \
        using TNode        = std::remove_cvref_t<std::tuple_element_t<0, CallbackArgs>>;           \
        static_assert(std::derived_from<TNode, Node>);                                             \
        static_assert(std::is_invocable_v<Callback, TNode &>);                                     \
                                                                                                   \
        if (dynamic_cast<TNode *>(this) == nullptr)                                                \
        {                                                                                          \
            throw std::runtime_error(                                                              \
                "Tried setting a node hook that takes an incompatible node type");                 \
        }                                                                                          \
                                                                                                   \
        auto adapted_callback = [callback = std::forward<Callback>(on_##name)](Node &node) mutable \
        { callback(static_cast<TNode &>(node)); };                                                 \
                                                                                                   \
        hooks_.on_##name = std::move(adapted_callback);                                            \
    }
    DEFINE_HOOK_SETTER(mount)
    DEFINE_HOOK_SETTER(ready)
    DEFINE_HOOK_SETTER(tick)
    DEFINE_HOOK_SETTER(unmount)
#undef DEFINE_HOOK_SETTER

    virtual ~Node()
    {
        // note: node cannot be unmounted here, as it implies a virtual call
        assert(!is_mounted() && "Tried to destroy a mounted node; unmount it first.");
    }

    class CompositionCursor;

  protected:
    virtual void
    on_mount_()
    {
    }

    virtual void
    on_ready_()
    {
    }

    virtual void
    tick_()
    {
    }

    virtual void
    on_unmount_()
    {
    }

    void
    die_()
    {
        [[unlikely]]
        if (!is_mounted())
        {
            return;
        }

        auto task = [this]
        {
            [[likely]]
            if (parent())
            {
                parent()->remove_child(name());
            }
            else
            {
                throw std::runtime_error("Tried to kill root game node.");
            }
        };

        game()->schedule(std::move(task));
    }

  private:
    using ChildrenMap_ = std::flat_map<std::string, std::shared_ptr<Node>, std::less<>>;

    std::string  name_     = "";      // empty placeholder; default name is set on mount if unset
    bool         enabled_  = true;    // disabled nodes don't tick
    Game        *game_     = nullptr; // the game this node is mounted to, or nullptr if unmounted
    Node        *parent_   = nullptr;
    ChildrenMap_ children_ = {};
    Hooks        hooks_    = {};

    void
    mount_to_(Game *game)
    {
        assert(!is_mounted());

        if (name_.empty())
        {
            name_ = default_name();
        }

        game_ = game;

        on_mount_();
        hooks_.on_mount ? hooks_.on_mount(*this) : void();

        for (auto child : children())
        {
            child->mount_to_(game);
        }

        on_ready_();
        hooks_.on_ready ? hooks_.on_ready(*this) : void();
    }

    void
    unmount_()
    {
        for (auto child : children())
        {
            child->unmount_();
        }

        on_unmount_();
        hooks_.on_unmount ? hooks_.on_unmount(*this) : void();

        game_ = nullptr;
    }

    friend class Game; // needed for Game to mount and unmount nodes.
};

// Stateful fluent interface for building a node tree.
class Node::CompositionCursor
{
    std::vector<Node *> stack_;

  public:
    template <class Root>
        requires std::derived_from<std::remove_cvref_t<Root>, Node>
    explicit CompositionCursor(Root &root)
    {
        stack_.push_back(&root);
    }

    CompositionCursor &
    add(std::shared_ptr<Node> child)
    {
        auto *parent = stack_.back();
        auto *node   = parent->add_child(std::move(child));
        stack_.push_back(node);
        return *this;
    }

    template <class T, class... Args>
        requires std::derived_from<T, Node>
    CompositionCursor &
    add(Args &&...args)
    {
        return add(std::make_shared<T>(std::forward<Args>(args)...));
    }

    CompositionCursor &
    named(std::string name)
    {
        stack_.back()->rename(std::move(name));

        return *this;
    }

#define DEFINE_HOOK_SETTER(name)                                                                   \
    CompositionCursor &on_##name(auto &&hook)                                                      \
    {                                                                                              \
        stack_.back()->hook_##name(std::forward<decltype(hook)>(hook));                            \
        return *this;                                                                              \
    }
    DEFINE_HOOK_SETTER(mount)
    DEFINE_HOOK_SETTER(ready)
    DEFINE_HOOK_SETTER(tick)
    DEFINE_HOOK_SETTER(unmount)
#undef DEFINE_HOOK_SETTER

    CompositionCursor &
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

[[nodiscard]] inline Node::CompositionCursor
extending(Node &root)
{
    return Node::CompositionCursor(root);
}

// Follows parent pointers up a game node tree until it finds the root node, which is returned.
inline Node &
find_root(Node *node)
{
    return node->parent() ? find_root(node->parent()) : *node;
}

template <class T>
    requires std::derived_from<T, Node>
inline T *
find_ancestor(Node *node)
{
    for (Node *parent = node->parent(); parent != nullptr; parent = parent->parent())
    {
        if (T *casted = dynamic_cast<T *>(parent))
        {
            return casted;
        }
    }
    return nullptr;
}

template <class T>
    requires std::derived_from<T, Node>
inline T *
find_descendant(Node *from)
{
    std::queue<Node *> queue;
    queue.push(from);

    while (!queue.empty())
    {
        Node *current = queue.front();
        queue.pop();

        for (auto child : current->children())
        {
            if (auto *casted = dynamic_cast<T *>(child))
            {
                return casted;
            }

            queue.push(child);
        }
    }

    return nullptr;
}

#define DEFINE_FIND_NAMED_RELATIVE(relation)                                                       \
    template <class T>                                                                             \
        requires std::derived_from<T, Node>                                                        \
    inline T *find_##relation(std::string_view name, Node *from)                                   \
    {                                                                                              \
        for (Node *it = from; it != nullptr; it = find_##relation<T>(it))                          \
        {                                                                                          \
            if (it->name() == name)                                                                \
            {                                                                                      \
                return it;                                                                         \
            }                                                                                      \
        }                                                                                          \
        return nullptr;                                                                            \
    }
DEFINE_FIND_NAMED_RELATIVE(ancestor)
DEFINE_FIND_NAMED_RELATIVE(descendant)
#undef DEFINE_FIND_NAMED_RELATIVE

/// Performs a depth-first traversal of a node tree,
// calling `pre` before visiting children and `post` after.
inline void
visit_dfs(Node &node, auto &&pre, auto &&post)
{
    std::invoke(pre, node);

    for (auto child : node.children())
    {
        visit_dfs(*child, pre, post);
    }

    std::invoke(post, node);
}

/// Performs a breadth-first traversal of a node tree,
// calling `pre` before visiting children and `post` after.
inline void
visit_bfs(Node &root, auto &&visit)
{
    std::queue<Node *> q;
    q.push(&root);

    while (!q.empty())
    {
        Node *node = q.front();
        q.pop();

        std::invoke(visit, *node);

        for (auto child : node->children())
        {
            q.push(child);
        }
    }
}

inline void
print_tree(Node &root)
{
    std::println("{}", root.name());

    auto recursive_step = [](auto &&self, Node &node, std::string prefix, bool is_last) -> void
    {
        std::print("{}{}", prefix, is_last ? "└─ " : "├─ ");
        std::println("{}", node.name());

        auto children = node.children();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            bool last = std::next(it) == children.end();

            self(self, **it, prefix + (is_last ? "   " : "│  "), last);
        }
    };

    auto children = root.children();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        bool last = std::next(it) == children.end();
        recursive_step(recursive_step, **it, "", last);
    }
}

// TODO: Use an actual logger approach instead of this
void
print_message(const auto &node, auto &&message)
{
    auto now  = std::chrono::system_clock::now();
    auto time = floor<std::chrono::seconds>(now);

    std::print("\033[37m[{:%H:%M:%S}]\033[0m "
               "\033[34m{} ({}): \033[0m "
               "\033[37m{}\033[0m\n",
               time,
               node.name(),
               node.default_name(),
               message);
}

} // namespace ome
