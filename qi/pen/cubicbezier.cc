#include "qi/pen/cubicbezier.h"

#include "qi.h"

namespace qi {
namespace pen {

template <typename T>
T CubicBezier<T>::Evaluate(float t) const {
  T tmp3[3];
  T tmp2[2];
  return Evaluate(t, tmp3, tmp2);
}

// TODO: inline makes performance difference?
template <typename T>
T CubicBezier<T>::Evaluate(float t, T* tmp3, T* tmp2) const {
  float one_minus_t = 1.0 - t;
  tmp3[0] = pts[0] * one_minus_t + pts[1] * t;
  tmp3[1] = pts[1] * one_minus_t + pts[2] * t;
  tmp3[2] = pts[2] * one_minus_t + pts[3] * t;
  tmp2[0] = tmp3[0] * one_minus_t + tmp3[1] * t;
  tmp2[1] = tmp3[1] * one_minus_t + tmp3[2] * t;
  return tmp2[0] * one_minus_t + tmp2[1] * t;
}

// TODO: test
// auto split = bez.split;
// for (t = 0.0; t <= 1.0; ++t) {
//   EXPECT_EQ(split.first.eval(t), bez.eval(t/2));
//   EXPECT_EQ(split.second.eval(t), bez.eval(t/2 + 0.5));
// }
template <typename T>
std::pair<CubicBezier<T>, CubicBezier<T>> CubicBezier<T>::Split(float t) const {
  T tmp3[3];
  T tmp2[2];
  T split_pt = Evaluate(t, tmp3, tmp2);

  std::pair<CubicBezier<T>, CubicBezier<T>> result;
  result.first.pts[0] = pts[0];
  result.first.pts[1] = tmp3[0];
  result.first.pts[2] = tmp2[0];
  result.first.pts[3] = split_pt;
  result.second.pts[0] = split_pt;
  result.second.pts[1] = tmp2[1];
  result.second.pts[2] = tmp3[2];
  result.second.pts[3] = pts[3];
  return result;
}

template <typename T>
CubicBezier<T> FitCubicBezier(T* pts, int count,
                              float* params,
                              float param_shift,
                              float param_scale,
                              T endpoint_tangent_0,
                              T endpoint_tangent_1) {
  float c00, c01, c10, c11;
  c00 = c01 = c10 = c11 = 0.0f;
  T x;

  for (int i = 0; i < count; ++i) {
    float t = (params[i] + param_shift) * param_scale;
    float omt = 1.0 - t;
    float b0 = omt * omt * omt;
    float b1 = 3.0 * t * omt * omt;
    float b2 = 3.0 * t * t * omt;
    float b3 = t * t * t;
    T a0 = endpoint_tangent_0 * b1;
    T a1 = endpoint_tangent_1 * b2;
    c00 += dot(a0, a0);
    c01 += dot(a0, a1);
    // c10 == dot(a1, a0) == c01, so don't compute here,
    // but instead set it just after the loop.
    c11 += dot(a1, a1);
    Pt2f tmp = pts[i] - (pts[0] * (b0 + b1) + pts[count - 1] * (b2 + b3));
    x[0] += dot(a0, tmp);
    x[1] += dot(a1, tmp);

  }

}

/*
CubicBezier1f FitCubicBezier1f(float* pts, int count,
                               float* params,
                               float param_shift,
                               float param_scale) {
  for (int i = 0; i < count; ++i) {
    float t = (params[i] + param_shift) * param_scale;
    float omt = 1.0 - t;
    float b0 = omt * omt * omt;
    float b1 = 3.0 * t * omt * omt;
    float b2 = 3.0 * t * t * omt;
    float b3 = t * t * t;
  }
}
 */

CubicBezier2f FitCubicBezier2f(Pt2f* pts,
                               int count,
                               float* params,
                               float param_shift,
                               float param_scale,
                               Pt2f endpoint_tangent_0,
                               Pt2f endpoint_tangent_1) {
  float c00, c01, c10, c11;
  c00 = c01 = c10 = c11 = 0.0f;
  Pt2f x;

#if 1
  // Non-vectorized version
  for (int i = 0; i < count; ++i) {
    float t = (params[i] + param_shift) * param_scale;
    float omt = 1.0 - t;
    float b0 = omt * omt * omt;
    float b1 = 3.0 * t * omt * omt;
    float b2 = 3.0 * t * t * omt;
    float b3 = t * t * t;
    Pt2f a0 = endpoint_tangent_0 * b1;
    Pt2f a1 = endpoint_tangent_1 * b2;
    c00 += dot(a0, a0);
    c01 += dot(a0, a1);
    // c10 == dot(a1, a0) == c01, so don't compute here,
    // but instead set it just after the loop.
    c11 += dot(a1, a1);
    Pt2f tmp = pts[i] - (pts[0] * (b0 + b1) + pts[count - 1] * (b2 + b3));
    x[0] += dot(a0, tmp);
    x[1] += dot(a1, tmp);
  }
#else  // VECTORIZE

  // Requires -ffast-math.  TODO: experiment with fp_contract pragma (maybe globally
  // enabling -ffast-math isn't necessary).

  float first_valx = pts[0].x;
  float first_valy = pts[0].y;
  float last_valx = pts[count-1].x;
  float last_valy = pts[count-1].y;

  float et0x = endpoint_tangent_0[0];
  float et0y = endpoint_tangent_0[1];
  float et1x = endpoint_tangent_1[0];
  float et1y = endpoint_tangent_1[1];

  float c00[2000];
  float c01[2000];
  float c11[2000];
  float x0[2000];
  float x1[2000];

#pragma clang loop vectorize(enable) interleave(enable)
  for (int i = 0; i < count; i++) {
    float t = (params[i] + param_shift) * param_scale;
    float omt = 1.0 - t;
    float b0 = omt * omt * omt;
    float b1 = 3.0 * t * omt * omt;
    float b2 = 3.0 * t * t * omt;
    float b3 = t * t * t;
    float b0b1 = b0 + b1;
    float b2b3 = b2 + b3;

    float a0x = et0x * b1;
    float a0y = et0y * b1;
    float a1x = et1x * b2;
    float a1y = et1y * b2;

    c00[i] = a0x * a0x + a0y * a0y;  // dot(a0, a0);
    c01[i] = a0x * a1x + a0y * a1y;  // dot(a0, a1);
    c11[i] = a1x * a1x + a1y * a1y;  // dot(a1, a1);
    float tmpx = pts[i*2] - first_valx * b0b1 - last_valx * b2b3;
    float tmpy = pts[i*2+1] - first_valy * b0b1 - last_valy * b2b3;
    x0[i] = a0x * tmpx + a0y * tmpy;  // dot(a0, tmp);
    x1[i] = a1x * tmpx + a1y * tmpy;  // dot(a1, tmp);
  }

  // Accumulate sums.  If we try to do this in the loop above, the vectorizer fails.
#pragma clang loop vectorize(enable) interleave(enable)
  for (int i = 0; i < count; i++) {
    c00 += c00[i];
    c01 += c01[i];
    c11 += c11[i];
    x[0] += x0[i];
    x[1] += x1[i];
  }
#endif  // VECTORIZE

  c10 = c01;

  float det_c0_c1 = c00  * c11  -  c10  * c01;
  float det_c0_x =  c00  * x[1] -  c01  * x[0];
  float det_x_c1 =  x[0] * c11  -  x[1] * c01;

  if (det_c0_c1 == 0.0)
    det_c0_c1 = c00 * c11 * 10e-12;

  // Compute alpha values used to determine the distance along the left/right tangent
  // vectors to place the middle two control points.  If either alpha value is negative,
  // recompute it using Wu/Barsky heuristic.
  float alpha_l = det_x_c1 / det_c0_c1;
  float alpha_r = det_c0_x / det_c0_c1;
  if (alpha_l < 0.0 || alpha_r < 0.0) {
    // Alpha was negative, so use Wu/Barsky heuristic to place points.
    // TODO: if only one alpha value is negative, should only that one be adjusted?
    alpha_l = alpha_r = distance(pts[0], pts[count-1]);
  }

  // Set all 4 control points and return the curve.
  CubicBezier2f fit;
  fit.pts[0] = pts[0];
  fit.pts[1] = pts[0] + endpoint_tangent_0 * alpha_l;
  fit.pts[2] = pts[count-1] + endpoint_tangent_1 * alpha_r;
  fit.pts[3] = pts[count-1];
  return fit;
}

std::pair<Pt2f, Pt2f> EvaluatePointAndNormal(
    const CubicBezier<Pt2f>& bez, float t) {
  Pt2f tmp3[3];
  Pt2f tmp2[2];
  Pt2f pt = bez.Evaluate(t, tmp3, tmp2);
  Pt2f tangent = normalize(tmp2[1] - tmp2[0]);

  // Rotate tangent clockwise by 90 degrees before returning.
  return std::make_pair(pt, Pt2f{-tangent.y, tangent.x});
}

// Force instantiation.
template class CubicBezier<float>;
template class CubicBezier<Pt2f>;

}  // namespace pen
}  // namespace qi
