#include <cmath>
#include <array>

class coord {
public:

  constexpr coord(float x, float y, float z)
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

private:
  std::array<float, 3> _coords;
};

inline coord operator+(const coord &left, const coord &right)
{
  return coord(left[0] + right[0], left[1] + right[1], left[2] + right[2]);
}

inline coord operator-(const coord &left, const coord &right)
{
  return coord(left[0] - right[0], left[1] - right[1], left[2] - right[2]);
}

inline coord operator*(const coord &coord_p, float scale)
{
  return coord(coord_p[0] * scale, coord_p[1] * scale, coord_p[2] * scale);
}

inline coord operator/(const coord &coord_p, float scale)
{
  return coord(coord_p[0] / scale, coord_p[1] / scale, coord_p[2] / scale);
}

inline float dot_product(const coord &left, const coord &right) 
{
  return
    left[0] * right[0] +
    left[1] * right[1] +
    left[2] * right[2];
}

inline coord cross_product(const coord &left, const coord &right)
{
  return coord(
    left[1] * right[2] - left[2] * right[1],
    left[2] * right[0] - left[0] * right[2],
    left[0] * right[1] - left[1] * right[0]);
}

inline float angle(const coord &left, const coord &right)
{
  return dot_product(left, right) / (left.length() * right.length());
}

inline coord normal(const coord &coord_p)
{
  return coord_p / coord_p.length();
}

inline coord operator-(const coord &coord_p)
{
  return coord(0,0,0) - coord_p;
}
