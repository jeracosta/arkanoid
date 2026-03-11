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

    explicit
    operator std::array<float, 3>() const
    {
        return coords_;
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

float
sqrlength(const Vector3 &vector)
{
    return vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2];
}

float
length(const Vector3 &vector)
{
    return std::sqrt(sqrlength(vector));
}

inline Vector3
operator+(const Vector3 &left, const Vector3 &right)
{
    return Vector3{
        left[0] + right[0],
        left[1] + right[1],
        left[2] + right[2],
    };
}

inline Vector3
operator-(const Vector3 &left, const Vector3 &right)
{
    return Vector3{
        left[0] - right[0],
        left[1] - right[1],
        left[2] - right[2],
    };
}

inline Vector3
operator*(const Vector3 &vector, float scale)
{
    return Vector3{
        vector[0] * scale,
        vector[1] * scale,
        vector[2] * scale,
    };
}

inline Vector3
operator/(const Vector3 &vector, float scale)
{
    return Vector3{
        vector[0] / scale,
        vector[1] / scale,
        vector[2] / scale,
    };
}

inline float
dot(const Vector3 &left, const Vector3 &right)
{
    return (left[0] * right[0]) + (left[1] * right[1]) + (left[2] * right[2]);
}

inline Vector3
cross(const Vector3 &left, const Vector3 &right)
{
    return Vector3(left[1] * right[2] - left[2] * right[1],
                   left[2] * right[0] - left[0] * right[2],
                   left[0] * right[1] - left[1] * right[0]);
}

inline float
angle(const Vector3 &left, const Vector3 &right)
{
    return dot(left, right) / (length(left) * length(right));
}

inline Vector3
operator-(const Vector3 &vector)
{
    return Vector3(0, 0, 0) - vector;
}

inline bool
operator==(const Vector3 &left, const Vector3 &right)
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
        assert(sqrlength(vector) == 1.0f);
    }

  public:
    friend UnitVector3
    normalize(const Vector3 &vector)
    {
        return { vector / length(vector) };
    }

    friend std::optional<UnitVector3>
    safe_normalize(const Vector3 &vector)
    {
        return (sqrlength(vector) != 0.0f) ? std::make_optional(normalize(vector)) : std::nullopt;
    }
};

float
sqrlength(const UnitVector3 &)
{
    return 1.0f;
}

float
length(const Vector3)
{
    return 1.0f;
}

Vector3
project(const Vector3 &vector, const Vector3 &onto)
{
    auto scalar_projection = dot(onto, vector) / sqrlength(onto);
    return onto * scalar_projection;
}

Vector3
project(const Vector3 &vector, const UnitVector3 &onto)
{
    return onto * dot(onto, vector);
}