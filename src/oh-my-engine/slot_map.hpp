#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ome {

template <typename TValue>
class GenerationalSlotMap
{
  public:
    struct Key
    {
        std::size_t index;
        uint8_t     generation;

        auto
        operator<=>(const Key &) const
            = default;
    };

    using Value = TValue;

  private:
    using Index = std::size_t;

    struct Slot
    {
        std::optional<Value> value;
        uint8_t              generation = 0;
    };

    std::vector<Slot> slots_;
    std::stack<Index> freed_indices_;
    std::size_t       size_ = 0;

    Key
    recycle_slot_(TValue &&value)
    {
        auto index = freed_indices_.top();
        freed_indices_.pop();

        auto &slot = slots_[index];

        slot.value.emplace(std::forward<TValue>(value));

        ++size_;

        return { index, slot.generation };
    }

    Key
    add_slot_(TValue &&value)
    {
        Index index = slots_.size();

        auto &&slot = Slot{
            .value      = std::forward<TValue>(value),
            .generation = 0,
        };

        slots_.emplace_back(slot);

        ++size_;

        return { index, slots_.back().generation };
    }

  public:
    bool
    contains(Key key) const
    {
        if (key.index >= slots_.size())
        {
            return false;
        }

        const auto &slot = slots_[key.index];

        return slot.value.has_value() && slot.generation == key.generation;
    }

    TValue &
    at(this auto &&self, Key key)
    {
        if (key.index >= self.slots_.size())
        {
            throw std::out_of_range("SlotMap: index out of range");
        }

        auto &slot = self.slots_[key.index];

        if (!slot.value || slot.generation != key.generation)
        {
            throw std::out_of_range("SlotMap: invalid/expired key");
        }

        return *slot.value;
    }

    TValue &
    operator[](this auto &&self, Key key)
    {
        return self.at(key);
    }

    std::size_t
    size() const
    {
        return size_;
    }

    Key
    emplace(TValue &&value)
    {
        if (!freed_indices_.empty())
        {
            return recycle_slot_(std::forward<TValue>(value));
        }

        return add_slot_(std::forward<TValue>(value));
    }

    void
    erase(Key key)
    {
        if (!contains(key))
            return;

        auto &slot = slots_[key.index];

        slot.value.reset();
        ++slot.generation;

        freed_indices_.push(key.index);
        --size_;
    }

    auto
    values(this auto &&self)
    {
        using namespace std::views;

        auto has_value = [](auto &slot) { return slot.value.has_value(); };
        auto get_value = [](auto &slot) -> Value & { return *slot.value; };

        return self.slots_ | filter(has_value) | transform(get_value);
    }
};

} // namespace ome
