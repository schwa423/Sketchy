#ifndef _QI_GFX_PEN_CubicBezier_h_
#define _QI_GFX_PEN_CubicBezier_h_

// TODO: serious cleanup

#import <CoreGraphics/CoreGraphics.h>

#include <Eigen/Core>
#include <utility>

#include <glm/glm.hpp>

namespace qi {
namespace pen {

using glm::length;
using glm::normalize;
using glm::dot;

typedef glm::tvec2<float> Pt2f;

// Compute distance between two points.
inline float distance(Pt2f a, Pt2f b) { return length(b - a); }

template <typename T>
struct CubicBezier {
  T pts[4];

  T Evaluate(float t) const;
  T Evaluate(float t, T* tmp3, T* tmp2) const;

  // Split into two curves at the specified parameter.
  std::pair<CubicBezier<T>, CubicBezier<T>> Split(float t) const;
};

typedef CubicBezier<float> CubicBezier1f;
typedef CubicBezier<Pt2f> CubicBezier2f;

template <typename T>
CubicBezier<T> FitCubicBezier(T* pts, int count,
                              float* params,
                              float param_shift,
                              float param_scale,
                              T endpoint_tangent_0,
                              T endpoint_tangent_1);

CubicBezier1f FitCubicBezier1f(float* pts, int count,
                               float* params,
                               float param_shift,
                               float param_scale);

CubicBezier2f FitCubicBezier2f(Pt2f* pts, int count,
                               float* params,
                               float param_shift,
                               float param_scale,
                               Pt2f endpoint_tangent_0,
                               Pt2f endpoint_tangent_1);

std::pair<Pt2f, Pt2f> EvaluatePointAndNormal(const CubicBezier<Pt2f>& bez, float t);

}  // namespace pen

template <typename T>
std::ostream& operator<<(std::ostream& os, const pen::Pt2f& pt) {
  return os << "(" << pt.x << "," << pt.y << ")";
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const pen::CubicBezier<T>& bez) {
  return os << "p0=" << bez.pts[0] << ", "
            << "p1=" << bez.pts[1] << ", "
            << "p2=" << bez.pts[2] << ", "
            << "p3=" << bez.pts[3];
}

}  // namespace qi

#endif  // _QI_GFX_PEN_CubicBezier_h_
