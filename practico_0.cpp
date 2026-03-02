#include <std:optional>

class coord
{
  public:
    float x, y, z;

  coord(float x, float y, float z) {
    length = _compute_length();
  }

  float length() {
    return _length;
  }
  
  float sqrlength();
  coord operator +(const coord &other) { return coord(x + other.x, y + other.y, z + other.z); }
  coord operator -(const coord &other) { return coord(x - other.x, y - other.y, z - other.z); }
  coord operator *(float scale) { return coord(x*scale, y*scale, z*sacalar); }
  coord operator /(float scale);
  coord dot_product (const coord &other);
  coord vec_product (const coord &other);
  coord angle (const coord &other);
  coord division(const coord &other);
  coord operator -(); //negate vector
  coord normalize();
  coord vec_product (const coord &other);
 
  private:
    const std::optional<float> _length = {};
    const compute_length();
}
