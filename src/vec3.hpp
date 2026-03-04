#include <cmath>
#include <array>

class vec3 
{
private:
  std::array<float, 3> _coords;

public:
  constexpr vec3(float x, float y, float z)
    : _coords{x, y, z}
  {
  }

  float length() const
  {
    return std::sqrt(sqrlength());
  }

  float sqrlength() const
  {
    return 
      _coords[0] * _coords[0] +
      _coords[1] * _coords[1] +
      _coords[2] * _coords[2];
  }

  float operator [](int i) const
  {
    return _coords[i];
  }

  operator std::array<float, 3>() const
  {
    return _coords;
  }
};

inline vec3 operator+(const vec3 &left, const vec3 &right)
{
  return vec3(left[0] + right[0],
              left[1] + right[1],
              left[2] + right[2]);
}

inline vec3 operator-(const vec3 &left, const vec3 &right)
{
  return vec3(left[0] - right[0],
              left[1] - right[1],
              left[2] - right[2]);
}

inline vec3 operator*(const vec3 &vector, float scale)
{
  return vec3(vector[0] * scale,
              vector[1] * scale,
              vector[2] * scale);
}

inline vec3 operator/(const vec3 &vector, float scale)
{
  return vec3(vector[0] / scale,
              vector[1] / scale,
              vector[2] / scale);
}

inline float dot_product(const vec3 &left, const vec3 &right) 
{
  return
    left[0] * right[0] +
    left[1] * right[1] +
    left[2] * right[2];
}

inline vec3 cross_product(const vec3 &left, const vec3 &right)
{
  return vec3(left[1] * right[2] - left[2] * right[1],
              left[2] * right[0] - left[0] * right[2],
              left[0] * right[1] - left[1] * right[0]);
}

inline float angle(const vec3 &left, const vec3 &right)
{
  return dot_product(left, right) / (left.length() * right.length());
}

inline vec3 normal(const vec3 &vector)
{
  return vector / vector.length();
}

inline vec3 operator-(const vec3 &vector)
{
  return vec3(0,0,0) - vector;
}
