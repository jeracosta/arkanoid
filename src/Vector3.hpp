#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>

class Vector3
{
  protected:
    std::array<float, 3> coords_;

  public:
    constexpr Vector3(float x, float y, float z)
        : coords_{ x, y, z }
    {
    }

    float
    length() const
    {
        return std::sqrt(sqrlength());
    }

    float
    sqrlength() const
    {
        return coords_[0] * coords_[0] + coords_[1] * coords_[1] + coords_[2] * coords_[2];
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
    return dot(left, right) / (left.length() * right.length());
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

class UnitVector3;

UnitVector3
normalize(const Vector3 &);

class UnitVector3 : public Vector3
{
  private:
    constexpr UnitVector3(const Vector3 &vector)
        : Vector3(vector)
    {
    }

  public:
    friend UnitVector3
    normalize(const Vector3 &vector)
    {
        return { vector / vector.length() };
    }

    friend std::optional<UnitVector3>
    safe_normalize(const Vector3 &vector)
    {
        if (vector.length() == 0.0f)
        {
            return std::nullopt;
        }
        return { normalize(vector) };
    }
};

inline bool
operator==(const Vector3 &left, const Vector3 &right)
{
    return std::ranges::equal(left, right);
}