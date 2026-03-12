#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

class UnitVector3;

class Vector3
{
  protected:
    std::array<float, 3> coords_;

  public:
    constexpr Vector3(float x, float y, float z)
        : coords_{ x, y, z }
    {
    }

    // FIXME: Pasar esto a proyección usando plano
    Vector3
    mirrorBy(const Vector3 &normal)
    {
        assert(normal == normalize(normal));
        return 2 * normal * dot_product(normal, *this) - *this;
    }

    float
    operator[](int i) const
    {
        return coords_[i];
    }

    decltype(auto) constexpr begin(this auto &&self) noexcept
    {
        return self.coords_.begin();
    }
    decltype(auto) constexpr end(this auto &&self) noexcept
    {
        return self.coords_.end();
    }
};

constexpr Vector3
operator+(const Vector3 &left, const Vector3 &right) noexcept
{
    return Vector3{
        left[0] + right[0],
        left[1] + right[1],
        left[2] + right[2],
    };
}

constexpr Vector3
operator-(const Vector3 &left, const Vector3 &right) noexcept
{
    return Vector3{
        left[0] - right[0],
        left[1] - right[1],
        left[2] - right[2],
    };
}

// TODO: Versiones de productos con escalar a la izquierda
// TODO: Asignment operators

constexpr Vector3
operator*(const Vector3 &vector, float scale) noexcept
{
    return Vector3{
        vector[0] * scale,
        vector[1] * scale,
        vector[2] * scale,
    };
}

constexpr Vector3
operator/(const Vector3 &vector, float scale)
{
    assert(scale != 0.0f);
    return Vector3{
        vector[0] / scale,
        vector[1] / scale,
        vector[2] / scale,
    };
}

constexpr float
dot(const Vector3 &left, const Vector3 &right) noexcept
{
    return (left[0] * right[0]) + (left[1] * right[1]) + (left[2] * right[2]);
}

constexpr Vector3
cross(const Vector3 &left, const Vector3 &right) noexcept
{
    return Vector3(left[1] * right[2] - left[2] * right[1],
                   left[2] * right[0] - left[0] * right[2],
                   left[0] * right[1] - left[1] * right[0]);
}

constexpr float
sqrlength(const Vector3 &vector)
{
    return dot(vector, vector);
}

constexpr float
length(const Vector3 &vector)
{
    return std::sqrt(sqrlength(vector));
}

constexpr float
angle(const Vector3 &left, const Vector3 &right)
{
    assert(length(left) != 0.0f && length(right) != 0.0f);
    return dot(left, right) / (length(left) * length(right));
}

constexpr Vector3
operator-(const Vector3 &vector) noexcept
{
    return Vector3(0, 0, 0) - vector;
}

constexpr bool
operator==(const Vector3 &left, const Vector3 &right) noexcept
{
    return std::ranges::equal(left, right);
}

UnitVector3
normalize(const Vector3 &);

class UnitVector3 : public Vector3
{
  private:
    constexpr UnitVector3(const Vector3 &vector)
        : Vector3(vector)
    {
        assert(sqrlength(vector) == 1.0f); // FIXME: assert is not constexpr
    }

  public:
    friend constexpr UnitVector3
    normalize(const Vector3 &vector)
    {
        return { vector / length(vector) };
    }

    friend constexpr std::optional<UnitVector3>
    safe_normalize(const Vector3 &vector)
    {
        return (sqrlength(vector) != 0.0f) ? std::make_optional(normalize(vector)) : std::nullopt;
    }
};

constexpr float
sqrlength(const UnitVector3 &) noexcept
{
    return 1.0f;
}

constexpr float
length(const Vector3) noexcept
{
    return 1.0f;
}

// FIXME: No usa la versión de UnitVector3. Usar polimorfismo estático
constexpr float
project_scalar(const Vector3 &vector, const Vector3 &onto)
{
    return dot(onto, vector) / sqrlength(onto);
}

constexpr Vector3
project(const Vector3 &vector, const Vector3 &onto)
{
    return onto * project_scalar(vector, onto);
}