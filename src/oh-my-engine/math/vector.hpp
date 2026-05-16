#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <glm/glm.hpp>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ome {

namespace math {

// #region Coordinate system

#define OME_COORDINATE_SYSTEMS(X)                                                                  \
    X(Cartesian)                                                                                   \
    X(Polar)                                                                                       \
    X(Spherical)

enum class CoordinateSystem
{
#define X(name) name,
    OME_COORDINATE_SYSTEMS(X)
#undef X
};

// #endregion

template <class>
inline constexpr bool is_vector_v = false;

template <class T>
concept is_vector = is_vector_v<std::remove_cvref_t<T>>;

template <std::size_t TDimension,
          typename TComponent     = float,
          CoordinateSystem TBasis = CoordinateSystem::Cartesian>
    requires(TDimension > 0)
class Vector
{
  private:
    std::array<TComponent, TDimension> components_;

    // #region Constructor helpers

    template <class T>
    static consteval std::size_t
    arg_arity_()
    {
        using A = std::remove_cvref_t<T>;
        if constexpr (is_vector_v<A>)
            return A::dimension();
        else
            return 1;
    }

    template <class... Args>
    static consteval std::size_t
    packed_args_arity_()
    {
        return (arg_arity_<Args>() + ... + 0);
    }

    template <class T>
    constexpr void
    append_one_(std::size_t &i, T &&x)
    {
        using A = std::remove_cvref_t<T>;
        if constexpr (is_vector_v<A>)
        {
            for (auto &&c : x)
            {
                components_[i++] = static_cast<TComponent>(c);
            }
        }
        else
        {
            components_[i++] = static_cast<TComponent>(x);
        }
    }

    template <typename Tuple, std::size_t... Is>
    constexpr Vector(Tuple &&tuple, std::index_sequence<Is...>)
        : components_{ static_cast<TComponent>(std::get<Is>(std::forward<Tuple>(tuple)))... }
    {
    }

    // #endregion

  public:
    using Component = TComponent;

    static consteval auto
    dimension()
    {
        return TDimension;
    }

    constexpr auto
    size() const
    {
        return dimension();
    }

    static consteval auto
    basis()
    {
        return TBasis;
    }

    // #region Constructors

    constexpr Vector() = default;

    template <typename... Args>
        requires(sizeof...(Args) > 0
                 && ((is_vector_v<std::remove_cvref_t<Args>>
                      || std::convertible_to<Args, TComponent>)
                     && ...)
                 && (packed_args_arity_<Args...>() == TDimension)
                 && (sizeof...(Args) != 1 || (is_vector_v<std::remove_cvref_t<Args>> || ...)))
    constexpr Vector(Args &&...args)
    {
        std::size_t i = 0;
        (append_one_(i, std::forward<Args>(args)), ...);
    }

    template <typename T>
        requires(std::convertible_to<T, Component>)
    constexpr Vector(const T value)
    {
        components_.fill(static_cast<TComponent>(value));
    }

    template <typename U, CoordinateSystem S>
        requires std::convertible_to<U, TComponent>
    explicit constexpr Vector(const Vector<TDimension, U, S> &other)
    {
        auto cast = [](auto v) { return static_cast<TComponent>(v); };
        std::ranges::transform(other, components_.begin(), cast);
    }

    template <std::ranges::input_range Range>
        requires(std::convertible_to<std::ranges::range_value_t<Range>, TComponent>)
    explicit constexpr Vector(Range &&range)
    {
        if (std::size(range) != dimension())
        {
            throw std::invalid_argument("Range size must match vector dimension");
        }

        std::ranges::copy(range, components_.begin());
    }

    template <typename... Ts>
        requires(sizeof...(Ts) == TDimension && (std::convertible_to<Ts, TComponent> && ...))
    constexpr Vector(const std::tuple<Ts...> &tuple)
        : Vector(tuple, std::index_sequence_for<Ts...>{})
    {
    }

    constexpr Vector(const Vector &other) noexcept = default;

    constexpr Vector(Vector &&other) noexcept = default;

    operator Component() const
        requires(TDimension == 1)
    {
        return components_[0];
    }

    // #endregion

    template <CoordinateSystem NewBasis>
    Vector<TDimension, TComponent, NewBasis>
    rebased() const
    {
        using enum CoordinateSystem;

        if constexpr (dimension() == 3)
        {
            if constexpr (TBasis == Cartesian && NewBasis == Spherical)
            {
                auto [x, y, z] = components_;
                auto r         = sqrt(x * x + y * y + z * z);
                auto phi       = acos(y / r);
                auto theta     = atan2(z, x);
                return { r, phi, theta };
            }
            else if constexpr (TBasis == Spherical && NewBasis == Cartesian)
            {
                auto [r, phi, theta] = components_;
                auto x               = r * sin(phi) * cos(theta);
                auto y               = r * cos(phi);
                auto z               = r * sin(phi) * sin(theta);
                return { x, y, z };
            }
        }
    }

    ~Vector() noexcept = default;

    inline decltype(auto) constexpr
    operator[](this auto &&self, size_t index)
    {
        if (index >= self.dimension())
        {
            throw std::out_of_range("Index out of bounds for Vector");
        }

        return self.components_[index];
    }

    template <std::size_t... Indices>
        requires(sizeof...(Indices) > 0 && ((Indices < TDimension) && ...))
    constexpr Vector<sizeof...(Indices), TComponent, TBasis>
    swizzled() const
    {
        return { components_[Indices]... };
    }

    decltype(auto) constexpr begin(this auto &&self) noexcept
    {
        return self.components_.begin();
    }
    decltype(auto) constexpr end(this auto &&self) noexcept
    {
        return self.components_.end();
    }

    friend void
    swap(Vector &lhs, Vector &rhs)
    {
        using std::swap;
        swap(lhs.components_, rhs.components_);
    }

    bool
    operator==(const Vector &other) const noexcept
    {
        return components_ == other.components_;
    }

    auto &
    operator=(Vector other)
    {
        // Copy-and-swap idiom
        swap(*this, other);
        return *this;
    }

#define BINARY_ASSIGNMENT_OPERATOR(op, operand_type, transformation)                               \
    constexpr Vector &operator op(const operand_type & operand)                                    \
    {                                                                                              \
        using std::ranges::transform;                                                              \
        if constexpr (std::same_as<operand_type, Vector>)                                          \
        {                                                                                          \
            transform(components_, operand.components_, components_.begin(), transformation<>());  \
            return *this;                                                                          \
        }                                                                                          \
        else if constexpr (std::same_as<operand_type, Component>)                                  \
        {                                                                                          \
            transform(components_,                                                                 \
                      components_.begin(),                                                         \
                      std::bind(transformation{}, std::placeholders::_1, operand));                \
            return *this;                                                                          \
        }                                                                                          \
    }
    BINARY_ASSIGNMENT_OPERATOR(+=, Vector, std::plus)
    BINARY_ASSIGNMENT_OPERATOR(-=, Vector, std::minus)
    BINARY_ASSIGNMENT_OPERATOR(*=, Vector, std::multiplies)
    BINARY_ASSIGNMENT_OPERATOR(/=, Vector, std::divides)
    BINARY_ASSIGNMENT_OPERATOR(*=, Component, std::multiplies)
    BINARY_ASSIGNMENT_OPERATOR(/=, Component, std::divides)
#undef BINARY_ASSIGNMENT_OPERATOR

    template <typename T>
    constexpr explicit
    operator Vector<TDimension, T>() const
    {
        return Vector<TDimension, T>((components_));
    }

#ifdef GLM_VERSION
    template <typename Component>
    constexpr Vector(const glm::vec<TDimension, Component> &v)
    {
        std::copy_n(&v[0], TDimension, components_.begin());
    }

    operator glm::vec<TDimension, Component>() const
    {
        glm::vec<TDimension, Component> glm_vector{};
        std::ranges::copy(components_, &glm_vector[0]);
        return glm_vector;
    }
#endif

#define X(name) using name = Vector<TDimension, Component, CoordinateSystem::name>;
    OME_COORDINATE_SYSTEMS(X)
#undef X
};

#ifdef GLM_VERSION
template <std::size_t Dimension, typename Component>
Vector(const glm::vec<Dimension, Component> &) -> Vector<Dimension, Component>;
#endif

template <std::size_t D, typename C, CoordinateSystem S>
inline constexpr bool is_vector_v<Vector<D, C, S>> = true;

// #region Free operators

constexpr auto
operator-(const is_vector auto &vector)
{
    using Vector = std::remove_cvref_t<decltype(vector)>;
    using namespace std::ranges;

    return vector | views::transform(std::negate<>{}) | to<Vector>();
}

#define BINARY_OPERATOR(op)                                                                        \
    [[nodiscard]] constexpr auto operator op(const is_vector auto &lhs, const auto &rhs)           \
    {                                                                                              \
        return auto(lhs) op## = rhs;                                                               \
    }                                                                                              \
    template <typename Scalar, is_vector Vector>                                                   \
        requires(!is_vector_v<Scalar>)                                                             \
    [[nodiscard]] constexpr auto operator op(const Scalar &lhs, const Vector &rhs)                 \
    {                                                                                              \
        return rhs op lhs;                                                                         \
    }
// point-point:
BINARY_OPERATOR(+)
BINARY_OPERATOR(-)
// point-scalar:
BINARY_OPERATOR(*)
BINARY_OPERATOR(/)
#undef BINARY_OPERATOR

// #endregion

// #region Utility functions

template <is_vector L, is_vector R, typename Comparator>
    requires std::same_as<std::invoke_result_t<Comparator,
                                               typename std::remove_cvref_t<L>::Component,
                                               typename std::remove_cvref_t<R>::Component>,
                          bool>
constexpr bool
component_wise(Comparator comparator, L &&lhs, R &&rhs)
{
    static_assert(lhs.dimension() == rhs.dimension());

    auto compare = [&comparator](auto pair)
    {
        auto &&[a, b] = pair;
        return comparator(a, b);
    };

    auto zipped = std::views::zip(lhs, rhs);

    return std::ranges::all_of(zipped, compare);
}

template <class Vector>
constexpr auto
dot(const Vector &lhs, const Vector &rhs)
{
    static_assert(lhs.dimension() == rhs.dimension(), "Vectors must have the same dimension");

    using namespace std::ranges;

    auto zipped = std::views::zip(lhs, rhs);

    auto product = [](auto acc, auto pair)
    {
        auto &&[a, b] = pair;
        return acc + a * b;
    };

    return std::accumulate(zipped.begin(), zipped.end(), 0.0, product);
}

constexpr auto
norm(const is_vector auto &vector)
{
    auto sum_square = [](auto acc, auto x) { return acc + x * x; };

    return std::sqrt(std::accumulate(std::begin(vector), std::end(vector), 0.0f, sum_square));
}

// TODO: Consider renaming to "normalize" for consistency with common terminology.
template <class Vector>
Vector
normalized(const Vector &vector)
{
    auto length = norm(vector);

    if (length == 0)
    {
        throw std::runtime_error("Cannot normalize a zero-length vector");
    }

    return vector / length;
}

template <class Vector>
Vector
is_normalized(const Vector &vector)
{
    return norm(vector) == 1;
}

template <typename Range>
Vector(Range &&range) -> Vector<decltype(std::ranges::size(range))::value,
                                typename std::ranges::range_value_t<Range>>;

template <typename... Args>
Vector(Args...) -> Vector<sizeof...(Args), std::common_type_t<Args...>>;

template <is_vector Vector>
constexpr auto
zip_transform(auto transformation, const Vector &lhs, const Vector &rhs)
{
    return Vector(std::views::zip_transform(transformation, lhs, rhs));
}

template <is_vector Vector>
constexpr auto
transform(const Vector &vector, auto transformation)
{
    return Vector(vector | std::views::transform(transformation));
}

template <is_vector Vector, class RandomNumberGenerator>
static Vector
random_vector(Vector from, Vector to, RandomNumberGenerator &&rng)
{
    Vector result;

    for (auto [r, f, t] : std::views::zip(result, from, to))
    {
        r = std::uniform_real_distribution{ f, t }(rng);
    }

    return result;
}

template <is_vector Vector>
Vector
projection(const Vector &vector, const Vector &onto)
{
    auto denom = dot(onto, onto);
    assert(denom > 0);

    return onto * (dot(vector, onto) / denom);
}

// Perpendicular component of vector relative to dir.
// dir must be a unit vector.
template <is_vector Vector>
Vector
reject(const Vector &vector, const Vector &dir)
{
    assert(norm(dir) == 1 && "Direction must be normalized for rejection");
    return vector - math::dot(vector, dir) * dir;
}

// Orthonormal projection onto the orthogonal complement of dir.
// Assumes dir is a unit vector.
template <is_vector Vector>
Vector
orthonormalize(const Vector &vector, const Vector &dir)
{
    Vector orthogonal = reject(vector, dir);

    if (norm(orthogonal) == 0)
    {
        return orthogonal;
    }

    return normalized(orthogonal);
}

// Muller method: N independent standard normals → normalise.
// Produces a uniform distribution on the (N-1)-sphere without rejection.
// O(N), works for any dimension.
template <is_vector Vector, class Rng>
Vector
random_unit_vector(Rng &rng)
{
    auto normal = std::normal_distribution<typename Vector::Component>{};

    Vector vector;
    for (;;)
    {
        for (auto &component : vector)
        {
            component = normal(rng);
        }

        if (norm(vector) > 0)
        {
            return math::normalized(vector);
        }
    }
}

template <is_vector Vector>
constexpr Vector
clamp(const Vector &value, const Vector &min, const Vector &max)
{
    return Vector(std::views::zip_transform(
        [](auto x, auto lo, auto hi) { return std::clamp(x, lo, hi); }, value, min, max));
}

// #endregion

} // namespace math

// #region Type aliases

#define DEFINE_VEC_N(N)                                                                            \
    template <typename Component                = float,                                           \
              ome::math::CoordinateSystem Basis = ome::math::CoordinateSystem::Cartesian>          \
    using Vec##N = ome::math::Vector<N, Component, Basis>;
DEFINE_VEC_N(1)
DEFINE_VEC_N(2)
DEFINE_VEC_N(3)
DEFINE_VEC_N(4)
#undef DEFINE_VEC_N

#define DEFINE_VEC_ALIAS(SUFFIX, TYPE)                                                             \
    using Vec1##SUFFIX = Vec1<TYPE>;                                                               \
    using Vec2##SUFFIX = Vec2<TYPE>;                                                               \
    using Vec3##SUFFIX = Vec3<TYPE>;                                                               \
    using Vec4##SUFFIX = Vec4<TYPE>;
DEFINE_VEC_ALIAS(f, float)
DEFINE_VEC_ALIAS(i, int)
DEFINE_VEC_ALIAS(u, unsigned)
DEFINE_VEC_ALIAS(b, bool)
#undef DEFINE_VEC_ALIAS

// #endregion

} // namespace ome

// #region Structured bindings

namespace std {

template <ome::math::is_vector Vector>
struct tuple_size<Vector> : integral_constant<std::size_t, Vector::dimension()>
{
};

template <std::size_t I, ome::math::is_vector Vector>
struct tuple_element<I, Vector>
{
    using type = typename Vector::Component;
};

} // namespace std

// The following are required to support structured bindings.
// e.g. auto [x, y, z] = Vec3f{1.0f, 2.0f, 3.0f};
namespace ome::math {

template <std::size_t I, std::size_t D, typename C, CoordinateSystem S>
constexpr C &
get(Vector<D, C, S> &v) noexcept
{
    static_assert(I < D);
    return v[I];
}

template <std::size_t I, std::size_t D, typename C, CoordinateSystem S>
constexpr const C &
get(const Vector<D, C, S> &v) noexcept
{
    static_assert(I < D);
    return v[I];
}

template <std::size_t I, std::size_t D, typename C, CoordinateSystem S>
constexpr C &&
get(Vector<D, C, S> &&v) noexcept
{
    static_assert(I < D);
    return std::move(v[I]);
}

template <std::size_t I, std::size_t D, typename C, CoordinateSystem S>
constexpr const C &&
get(const Vector<D, C, S> &&v) noexcept
{
    static_assert(I < D);
    return std::move(v[I]);
}

// #endregion

} // namespace ome::math
