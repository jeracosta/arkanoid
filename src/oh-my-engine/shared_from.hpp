#include <memory>

namespace ome {

template <class T>
std::shared_ptr<T>
shared_from(T *pointer)
{
    return std::static_pointer_cast<T>(pointer->shared_from_this());
}

} // namespace ome
