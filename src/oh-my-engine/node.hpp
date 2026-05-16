// A node is the basic building block of a game.
// Nodes are "mounted" to become part of a game and begin affecting it in some way.
// Users can derive from a node to implement their own game logic.
// Games tick all mounted nodes every frame, traversing the tree in a depth-first manner.
// Nodes can "die": a dead node doesn't tick, and gets unmounted and freed by the game.
// Additional specialized systems (e.g. rendering) could interact with nodes as well.
// A node can be mounted in three ways:
//   1. A game assigns it as the root node.
//   2. Another mounted node adds it as a child.
//   3. Its parent is mounted, recursively mounting all descendants.
// Unmounting a node unmounts all of its children as well.
// When recursively mounting a tree (node), nodes became "ready" when no children are left to mount.
// Nodes call virtual lifecycle hooks on: mount (top-down), ready (bot-up), and unmount (bot-up).
// Each lifecycle phase also emits a corresponding event, which users can bind callbacks to.

#pragma once

// #region Includes

#include <boost/type_index.hpp>
#include <cassert>
#include <flat_map>
#include <functional>
#include <memory>
#include <queue>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include "copy_const.hpp"
#include "game.hpp"

// #endregion

namespace ome {

// #region Lifecycle events
// clang-format off

struct NodeMounted {};
struct NodeGotReady {};
struct NodeTicked {};
struct NodeUnmounted {};

// #endregion
// clang-format on

class Node : public std::enable_shared_from_this<Node>,
             private EventBus<NodeMounted, NodeGotReady, NodeTicked, NodeUnmounted>,
             public EventConnectionHolder
{
  public:
    enum LifecyclePhase
    {
        Unmounted,      // Before first mount, or after unmounting
        Mounting,       // During mount recursion (downward)
        Mounted,        // Got ready and it's tickable
        PendingUnmount, // Between calling die() and the end-of-frame cleanup
        Unmounting,     // During unmount recursion (upward)
    };

    Node(std::string_view name = {})
        : name_(name)
    {
    }

    // NOTE: Nodes are not movable to avoid dangling pointers in parent/children relationships

    Node(Node &&) = delete;

    Node &
    operator=(Node &&)
        = delete;

    // #region Properties

    std::string_view
    name() const
    {
        return name_;
    }

    auto *
    game(this auto &&self)
    {
        return self.game_;
    }

    auto *
    parent(this auto &&self)
    {
        return self.parent_;
    }

    LifecyclePhase
    lifecycle_phase() const
    {
        return phase_;
    }

    bool
    is_unmounted() const
    {
        return phase_ == Unmounted;
    }

    bool
    is_ready() const
    {
        return phase_ == Mounted;
    }

    std::string
    default_name() const
    {
        return boost::typeindex::type_id_runtime(*this).pretty_name() + "@" + address_string(this);
    }

    Node &
    rename(std::string new_name)
    {
        [[unlikely]]
        if (new_name == name_)
        {
            return *this;
        }

        [[unlikely]]
        if (is_in_transition_())
        {
            // We defer it to prevent children iterator invalidation

            auto &&task = [this, name = std::move(new_name)]() mutable { rename(std::move(name)); };
            game_->schedule(task);

            return *this;
        }

        [[unlikely]]
        if (!parent_)
        {
            name_ = std::move(new_name);
            return *this;
        }

        [[unlikely]]
        if (parent_->children_.contains(new_name))
        {
            throw std::runtime_error(
                std::format("Tried renaming node '{}' to sibling name '{}'", name_, new_name));
        }

        auto it   = parent_->children_.find(name_);
        auto node = std::move(it->second);
        parent_->children_.erase(it);

        name_ = std::move(new_name);
        parent_->children_.emplace(name_, std::move(node));

        return *this;
    }

    // #endregion

    // #region Tree-related methods

    auto
    children(this auto &&self)
    {
        return self.children_ // view over its stored pointers:
               | std::views::values | std::views::transform(&std::shared_ptr<Node>::get);
    }

    template <class TChild>
        requires std::derived_from<TChild, Node>
    TChild &
    add_child(std::shared_ptr<TChild> child_owner)
    {
        auto child = child_owner.get();

        [[unlikely]]
        if (child->parent_)
        {
            throw std::runtime_error(std::format(
                "Node '{}' tried reassigning the parent of node '{}'", name(), child->name()));
        }

        [[unlikely]]
        if (!child->is_unmounted())
        {
            throw std::runtime_error(std::format(
                "Node '{}' tried mounting the already mounted node '{}'", name(), child->name()));
        }

        [[unlikely]]
        if (is_in_transition_())
        {
            // We defer it to prevent children iterator invalidation

            auto &&task = [this, child_owner = std::move(child_owner)]() mutable
            { //
                add_child(std::move(child_owner));
            };

            game_->schedule(task);

            return *child;
        }

        [[unlikely]]
        if (children_.contains(child->name_))
        {
            throw std::runtime_error(std::format(
                "Node '{}' tried adding a child with colliding name '{}'", name(), child->name()));
        }

        child->parent_ = this;
        children_.emplace(child->name_, std::move(child_owner));

        [[likely]]
        if (game_ != nullptr)
        {
            child->mount_to_(game_);
        }

        return *child;
    }

    template <class TChild, class... Args>
        requires std::derived_from<TChild, Node>
    TChild &
    emplace_child(Args &&...args)
    {
        return add_child(std::make_shared<TChild>(std::forward<Args>(args)...));
    }

    std::shared_ptr<Node>
    remove_child(const std::string_view &name)
    {
        auto it = children_.find(name);

        [[unlikely]]
        if (it == children_.end())
        {
            return nullptr;
        }

        if (is_in_transition_())
        {
            // We defer it to prevent children iterator invalidation

            auto target = it->second;

            auto &&task = [this, target = std::string(name)] { remove_child(target); };
            game_->schedule(task);

            return target;
        }

        auto child = std::move(it->second);
        children_.erase(it);

        if (!child->is_unmounted())
        {
            child->unmount_();
        }

        child->parent_ = nullptr;
        return child;
    }

    // #endregion

    // #region Tick and lifecycle management

    void
    tick()
    {
        [[unlikely]]
        if (!is_ready())
        {
            return;
        }

        on_tick_();

        emit(NodeTicked{});
    }

    // Schedules the node for unmounting at the end of the current frame. The node will stop ticking
    void
    schedule_unmount()
    {
        [[unlikely]]
        if (is_unmounted() || phase_ == PendingUnmount)
        {
            return;
        }

        phase_ = PendingUnmount;

        game()->schedule([this]
        {
            if (parent())
            {
                parent()->remove_child(name());
            }
            else
            {
                throw std::runtime_error(std::format("Tried to schedule unmounting of root node "
                                                     "'{}'. Root nodes cannot be unmounted,",
                                                     name()));
            }
        });
    }

    void
    log(const auto &message, LogLevel level = LogLevel::Info)
    {
        assert(is_mounted() && "Tried logging from an unmounted node; mount it first.");

        auto prefix = std::format("\033[34m{} ({}): \033[0m", name(), address_string(this));

        game()->logger().log(prefix + message, level);
    }

    virtual ~Node()
    {
        assert(is_unmounted() && "Node destroyed while still mounted!");
    }

    using EventBus::bind;

    class CompositionCursor;

    // #endregion

  protected:
    // #region Lifecycle hooks
    // clang-format off

    virtual void  on_mount_   (){} // Can safely modify tree structure. Changes will be scheduled.
    virtual void  on_ready_   (){} // WARN: Do NOT modify tree structure in this hook (SIGSEV risk).
    virtual void  on_tick_    (){}
    virtual void  on_unmount_ (){}

    // #endregion
    // clang-format on

  private:
    // #region Member variables

    using ChildrenMap_ = std::flat_map<std::string, std::shared_ptr<Node>, std::less<>>;

    std::string    name_     = ""; // If left empty, it will be defaulted on mount
    LifecyclePhase phase_    = Unmounted;
    Game          *game_     = nullptr;
    Node          *parent_   = nullptr;
    ChildrenMap_   children_ = {};

    // #endregion

    // #region Mount/Unmount logic

    bool
    is_in_transition_() const
    {
        return phase_ == Mounting || phase_ == Unmounting;
    }

    void
    mount_to_(Game *game)
    {
        assert(phase_ == Unmounted);

        game_  = game;
        phase_ = Mounting;

        if (name_.empty())
        {
            name_ = default_name();
        }

        on_mount_();

        emit(NodeMounted{});

        for (auto child : children())
        {
            child->mount_to_(game);
        }

        phase_ = Mounted;

        on_ready_();

        emit(NodeGotReady{});
    }

    void
    unmount_()
    {
        assert(is_mounted());

        phase_ = Unmounting;

        // Unmount children first (down-up order)
        for (auto child : children())
        {
            child->unmount_();
        }

        on_unmount_();
        emit(NodeUnmounted{});

        game_  = nullptr;
        phase_ = Unmounted;
    }

    // #endregion

    friend class Game; // allows Game to mount and unmount nodes.
};

// #region Tree traversal utilities

// Follows parent pointers up a game node tree until it finds the root node, which is returned.
template <class TNode>
inline auto &
find_root(TNode *node)
{
    return node->parent() ? find_root(node->parent()) : *node;
}

template <class TAncestor, class TFrom>
    requires std::derived_from<TAncestor, Node>
inline copy_const_t<TFrom, TAncestor> *
find_ancestor(TFrom *node)
{
    assert(node);

    for (auto *it = node ? node->parent() : nullptr; it; it = it->parent())
    {
        if (auto *casted = dynamic_cast<copy_const_t<TFrom, TAncestor> *>(it))
        {
            return casted;
        }
    }

    return nullptr;
}

template <class TDescendant, class TFrom>
    requires std::derived_from<TDescendant, Node> && std::derived_from<TFrom, Node>
inline copy_const_t<TFrom, TDescendant> *
find_descendant(TFrom *from)
{
    if (!from)
    {
        return nullptr;
    }

    std::queue<copy_const_t<TFrom, Node> *> queue;
    queue.push(from);

    while (!queue.empty())
    {
        auto *current = queue.front();
        queue.pop();

        for (auto *child : current->children())
        {
            if (auto *casted = dynamic_cast<copy_const_t<TFrom, TDescendant> *>(child))
            {
                return casted;
            }

            queue.push(child);
        }
    }

    return nullptr;
}

#define DEFINE_FIND_NAMED_RELATIVE(relation)                                                       \
    template <class TRelative, class TFrom>                                                        \
        requires std::derived_from<TRelative, Node> && std::derived_from<TFrom, Node>              \
    inline copy_const_t<TFrom, TRelative> *find_##relation(std::string_view name, TFrom *from)     \
    {                                                                                              \
        for (auto *it = find_##relation<TRelative>(from); it != nullptr;                           \
             it       = find_##relation<TRelative>(it))                                            \
        {                                                                                          \
            if (it->name() == name)                                                                \
                return static_cast<copy_const_t<TFrom, TRelative> *>(it);                          \
        }                                                                                          \
        return static_cast<copy_const_t<TFrom, TRelative> *>(nullptr);                             \
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

// #endregion

// #region Debug utilities

inline std::string
tree_string(Node &root)
{
    std::string out;

    auto append_line = [&](std::string_view prefix, std::string_view name)
    {
        out += prefix;
        out += name;
        out += '\n';
    };

    append_line("", root.name());

    auto recursive_step = [&](auto &&self, Node &node, std::string prefix, bool is_last) -> void
    {
        append_line(prefix, std::string{ is_last ? "└─ " : "├─ " } + node.name());

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

    return out;
}

// #endregion

} // namespace ome
