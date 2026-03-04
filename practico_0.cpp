#include <optional>
#include <cmath>
#include <array>

class coord {
public:

  coord(float x, float y, float z)
  {
    _coords = {x,y,z};
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

  coord operator+(const coord &other) const
  {
    return coord(_coords[0] + other[0], _coords[1] + other[1], _coords[2] + other[2]);
  }

  coord operator-(const coord &other) const
  {
    return coord(_coords[0] - other[0], _coords[1] - other[1], _coords[2] - other[2]);
  }

  coord operator*(float scale) const
  {
    return coord(_coords[0] * scale, _coords[1] * scale, _coords[2] * scale);
  }

  coord operator/(float scale) const
  {
    return coord(_coords[0] / scale, _coords[1] / scale, _coords[2] / scale);
  }

  float dot_product(const coord &other) const 
  {
    return
      _coords[0] * other[0] +
      _coords[1] * other[1] +
      _coords[2] * other[2];
  }

  float operator [](int i) const
  {
    return _coords[i];
  }

  coord vec_product(const coord &other) const
  {
    return coord(
      _coords[1] * other[2] - _coords[2] * other[1],
      _coords[2] * other[0] - _coords[0] * other[2],
      _coords[0] * other[1] - _coords[1] * other[0]);
  }

  float angle(const coord &other) const
  {
    return dot_product(other) / (length() * other.length());
  }

  // TODO:
  // coord division(const coord &other);

  coord operator-() const
  {
    return coord(0,0,0) - *this;
  }   

  coord normalize() const 
  {
    return *this / length();
  }

private:
  std::array<float, 3> _coords;
};
