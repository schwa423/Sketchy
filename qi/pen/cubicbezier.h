#ifndef _QI_GFX_PEN_CubicBezier_h_
#define _QI_GFX_PEN_CubicBezier_h_

// TODO: serious cleanup

#import <CoreGraphics/CoreGraphics.h>

#include <Eigen/Core>

namespace qi {
namespace pen {

typedef Eigen::Vector2d Pt2d;  // typedef Matrix< double, 2, 1 >  (column vector)

typedef Eigen::Matrix2f Mat2f;

//    typedef Eigen::Vector2f Pt2f;
struct Pt2f {
  float x;
  float y;
  Pt2f() {}  // uninitialized  // TODO use NaN in debug mode.
  Pt2f(float xx, float yy) { x = xx; y = yy; }
  Pt2f(const CGPoint& pt) { x = pt.x; y = pt.y; }
  Pt2f(const Eigen::Vector2f pt) { x = pt[0]; y = pt[1]; }
  operator Eigen::Vector2f&() {
    return *reinterpret_cast<Eigen::Vector2f*>(this);
  }
  operator const Eigen::Vector2f&() const {
    return *reinterpret_cast<const Eigen::Vector2f*>(this);
  }
  Pt2f operator+(const Pt2f& other) const { return Pt2f(x + other.x, y + other.y); }
  Pt2f operator-(const Pt2f& other) const { return Pt2f(x - other.x, y - other.y); }
  Pt2f operator*(float scale) const { return Pt2f(x * scale, y * scale); }
  float& operator[](int index) { return (&(this->x))[index]; }
  bool operator==(const Pt2f& other) const { return x == other.x && y == other.y; }
  float dot(const Pt2f& other) const { return x * other.x + y * other.y; }
  float dist(const Pt2f& other) const { Pt2f vec = other - *this; return sqrt(vec.dot(vec)); }
  void normalize() {
      float inverse_length = 1.0f / sqrt(this->dot(*this));
      x *= inverse_length;
      y *= inverse_length;
  }

  static Pt2f Zero() { return Pt2f(0.0f, 0.0f); }
};

typedef Eigen::Vector4f Vec4f;
typedef Eigen::Array<float, 1, 4> Arr4f;
typedef Eigen::Array<float, 1, 8> Arr8f;

// Compute distance between two points.
inline float dist(Pt2f a, Pt2f b) {
  Pt2f diff = b - a;
  return sqrt(diff.dot(diff));
}

struct CubicBezier2f {
  Pt2f pts[4];

  Pt2f Evaluate(float t);
  Pt2f Evaluate(float t, Pt2f* tmp3, Pt2f* tmp2);
  std::pair<Pt2f, Pt2f> EvaluatePointAndNormal(float t);
  void Print();
  static CubicBezier2f Fit(Pt2f* pts,
                           int count,
                           float* params,
                           float param_shift,
                           float param_scale,
                           Pt2f endpoint_tangent_0,
                           Pt2f endpoint_tangent_1);
};

}  // namespace pen
}  // namespace qi

#endif  // _QI_GFX_PEN_CubicBezier_h_
