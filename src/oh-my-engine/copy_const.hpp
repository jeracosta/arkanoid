#include <type_traits>

template <class From, class To>
using copy_const_t = std::conditional_t<std::is_const_v<std::remove_cvref_t<From>>, const To, To>;
