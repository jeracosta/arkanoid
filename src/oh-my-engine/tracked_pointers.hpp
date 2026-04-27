#include <memory>

#include "object_registry.hpp"

namespace ome {

template <typename Pointer, typename T>
class Tracked;

template <typename T>
class Tracked<std::unique_ptr<T>, T>
{
  private:
    ObjectRegistry     &registry_;
    ObjectRegistry::Key key_;
    std::unique_ptr<T>  ptr_{};

    void
    register_()
    {
        if (ptr_)
        {
            key_ = registry_.insert(ptr_.get());
        };
    }

    void
    unregister_()
    {
        if (ptr_)
        {
            registry_.remove(key_);
            key_ = {};
        }
    }

  public:
    // Mirror std::unique_ptr's type aliases
    using pointer      = T *;
    using element_type = T;
    using deleter_type = std::default_delete<T>;

    Tracked(ObjectRegistry &r) noexcept
        : registry_(r)
    {
    }

    Tracked(ObjectRegistry &r, std::unique_ptr<T> &&p) noexcept
        : registry_(r),
          ptr_(std::move(p))
    {
        register_();
    }

    // non-copyable
    Tracked(const Tracked &) = delete;
    Tracked &
    operator=(const Tracked &)
        = delete;

    // movable
    Tracked(Tracked &&other) noexcept
        : registry_(other.registry_),
          key_(other.key_),
          ptr_(std::move(other.ptr_))
    {
        other.key_ = {};
    }

    Tracked &
    operator=(Tracked &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            ptr_       = std::move(other.ptr_);
            key_       = other.key_;
            other.key_ = {};
        }
        return *this;
    }

    ~Tracked()
    {
        unregister_();
    }

    T *
    get() const noexcept
    {
        return ptr_.get();
    }

    T &
    operator*() const noexcept
    {
        return *ptr_;
    }

    T *
    operator->() const noexcept
    {
        return ptr_.get();
    }

    explicit
    operator bool() const noexcept
    {
        return (bool)ptr_;
    }

    T *
    release() noexcept
    {
        unregister_();
        return ptr_.release();
    }

    void
    reset(T *p = nullptr) noexcept
    {
        unregister_();
        ptr_.reset(p);
        register_();
    }

    void
    swap(Tracked &other) noexcept
    {
        std::swap(ptr_, other.ptr_);
        std::swap(key_, other.key_);
    }

    std::unique_ptr<T> &
    raw() noexcept
    {
        return ptr_;
    }

    const std::unique_ptr<T> &
    raw() const noexcept
    {
        return ptr_;
    }
};

} // namespace ome
