// Macro to be used in class definition to forward method calls to a member variable.
#define FORWARD_TO_MEMBER(method, member)                                                          \
    template <class... Args>                                                                       \
    decltype(auto) method(this auto &&self, Args &&...args) noexcept(                              \
        noexcept(self.member.method(std::forward<Args>(args)...)))                                 \
    {                                                                                              \
        return self.member.method(std::forward<Args>(args)...);                                    \
    }
