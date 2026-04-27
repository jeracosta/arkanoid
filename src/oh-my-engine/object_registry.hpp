#include <any>
#include <mutex>
#include <type_traits>

#include "slot_map.hpp"

namespace ome {

class ObjectRegistry
{
  private:
    std::mutex                    mutex_;
    GenerationalSlotMap<std::any> object_pointers_;

  public:
    using Key = decltype(object_pointers_)::Key;

    template <typename T>
        requires std::is_pointer_v<T>
    Key
    insert(T object_pointer)
    {
        std::lock_guard lock(mutex_);
        return object_pointers_.emplace(std::move(object_pointer));
    }

    template <typename T>
        requires std::is_pointer_v<T>
    T
    query(const Key &key)
    {
        std::lock_guard lock(mutex_);

        if (!object_pointers_.contains(key))
        {
            return nullptr;
        }

        return std::any_cast<T>(object_pointers_.at(key));
    }

    template <typename T>
        requires std::is_pointer_v<T>
    void
    update(const Key &key, T object_pointer)
    {
        std::lock_guard lock(mutex_);
        return object_pointers_.at(key) = std::move(object_pointer);
    }

    void
    remove(const Key &key)
    {
        std::lock_guard lock(mutex_);
        return object_pointers_.erase(key);
    }
};

} // namespace ome
