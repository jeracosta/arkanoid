#include <bitset>
#include <cstdint>
#include <utility>

// User-defined actions, e.g. "InputAction::Jump". Defined in application code, not library code.
// XXX: must not exceed SUPPORTED_ACTIONS_COUNT, otherwise behavior is undefined.
enum class Action : uint8_t;

// Maximum number of actions supported by the system.
// Defined as macro to allow customization by application code;
// if sufficient, it should fit in L1 cache line for optimal performance.
#define SUPPORTED_ACTIONS_COUNT 64

using ActionMask = std::bitset<SUPPORTED_ACTIONS_COUNT>;

constexpr ActionMask
mask(Action a) noexcept
{
    return 1ull << std::to_underlying(a);
}

using ActionState = bool;

class ActionsState
{
  private:
    ActionMask active_actions_;

  public:
    ActionsState(auto &&active_actions)
        : active_actions_(std::forward<decltype(active_actions)>(active_actions))
    {
    }

    ActionState
    operator[](Action action) const
    {
        return active_actions_[std::to_underlying(action)];
    }
};
