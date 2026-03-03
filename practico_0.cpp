#include <optional>

class coord {
public:
  float x, y, z;

  coord(float x, float y, float z) : x(x), y(y), z(z) {}

  float length() {
    if (!_length_cache)
      _length_cache = _compute_length();
    return *_length_cache;
  }

  float length() const { return _compute_length(); }

  float sqrlength();

  coord operator+(const coord &other) {
    return coord(x + other.x, y + other.y, z + other.z);
  }

  coord operator-(const coord &other) {
    return coord(x - other.x, y - other.y, z - other.z);
  }

  coord operator*(float scale) {
    return coord(x * scale, y * scale, z * scale);
  }

  coord operator/(float scale);

  coord dot_product(const coord &other);

  coord vec_product(const coord &other);

  coord angle(const coord &other);

  coord division(const coord &other);

  // negate vector
  coord operator-();

  coord normalize();

private:
  std::optional<float> _length_cache = {};
  float _compute_length() const;
};
