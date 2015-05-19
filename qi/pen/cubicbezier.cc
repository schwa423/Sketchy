#include "qi/pen/cubicbezier.h"

#include <iostream>

namespace qi {
namespace pen {

Pt2f CubicBezier2f::Evaluate(float t) {
  Pt2f tmp3[3];
  Pt2f tmp2[2];
  return Evaluate(t, tmp3, tmp2);
}

Pt2f CubicBezier2f::Evaluate(float t, Pt2f* tmp3, Pt2f* tmp2) {
  float one_minus_t = 1.0 - t;
  tmp3[0] = pts[0] * one_minus_t + pts[1] * t;
  tmp3[1] = pts[1] * one_minus_t + pts[2] * t;
  tmp3[2] = pts[2] * one_minus_t + pts[3] * t;
  tmp2[0] = tmp3[0] * one_minus_t + tmp3[1] * t;
  tmp2[1] = tmp3[1] * one_minus_t + tmp3[2] * t;
  return tmp2[0] * one_minus_t + tmp2[1] * t;
}

std::pair<Pt2f, Pt2f> CubicBezier2f::EvaluatePointAndNormal(float t) {
  Pt2f tmp3[3];
  Pt2f tmp2[2];
  Pt2f pt = Evaluate(t, tmp3, tmp2);
  Pt2f tangent = (tmp2[1] - tmp2[0]);
  tangent.normalize();
  // Rotate tangent clockwise by 90 degrees before returning.
  return std::make_pair(pt, Pt2f{-tangent.y, tangent.x});
}

void CubicBezier2f::Print() {
  std::cerr << "p0(" << pts[0].x << "," << pts[0].y << "), "
            << "p1(" << pts[1].x << "," << pts[1].y << "), "
            << "p2(" << pts[2].x << "," << pts[2].y << "), "
            << "p3(" << pts[3].x << "," << pts[3].y << ")" << std::endl;
}

CubicBezier2f CubicBezier2f::Fit(Pt2f* pts,
                                 int count,
                                 float* params,
                                 float param_shift,
                                 float param_scale,
                                 Pt2f endpoint_tangent_0,
                                 Pt2f endpoint_tangent_1) {
  Mat2f c = Mat2f::Zero();
  Pt2f x = Pt2f::Zero();

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
    c(0,0) += a0.dot(a0);
    c(0,1) += a0.dot(a1);
    // c(1,0) == a1.dot(a0) == c(0,1), so don't compute here,
    // but instead set it just after the loop.
    c(1,1) += a1.dot(a1);
    Pt2f tmp = pts[i] - (pts[0] * (b0 + b1) + pts[count - 1] * (b2 + b3));
    x[0] += a0.dot(tmp);
    x[1] += a1.dot(tmp);
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

    c00[i] = a0x * a0x + a0y * a0y;  // a0.dot(a0);
    c01[i] = a0x * a1x + a0y * a1y;  // a0.dot(a1);
    c11[i] = a1x * a1x + a1y * a1y;  // a1.dot(a1);
    float tmpx = pts[i*2] - first_valx * b0b1 - last_valx * b2b3;
    float tmpy = pts[i*2+1] - first_valy * b0b1 - last_valy * b2b3;
    x0[i] = a0x * tmpx + a0y * tmpy;  // a0.dot(tmp);
    x1[i] = a1x * tmpx + a1y * tmpy;  // a1.dot(tmp);
  }

  // Accumulate sums.  If we try to do this in the loop above, the vectorizer fails.
#pragma clang loop vectorize(enable) interleave(enable)
  for (int i = 0; i < count; i++) {
    c(0,0) += c00[i];
    c(0,1) += c01[i];
    c(1,1) += c11[i];
    x[0] += x0[i];
    x[1] += x1[i];
  }
#endif  // VECTORIZE

  c(1,0) = c(0,1);

  float det_c0_c1 = c(0,0) * c(1,1)  -  c(1,0) * c(0,1);
  float det_c0_x =  c(0,0) * x[1]    -  c(0,1) * x[0];
  float det_x_c1 =  x[0]   * c(1,1)  -  x[1]   * c(0,1);

  if (det_c0_c1 == 0.0)
    det_c0_c1 = c(0,0) * c(1,1) * 10e-12;

  // Compute alpha values used to determine the distance along the left/right tangent
  // vectors to place the middle two control points.  If either alpha value is negative,
  // recompute it using Wu/Barsky heuristic.
  float alpha_l = det_x_c1 / det_c0_c1;
  float alpha_r = det_c0_x / det_c0_c1;
  if (alpha_l < 0.0 || alpha_r < 0.0) {
    // Alpha was negative, so use Wu/Barsky heuristic to place points.
    // TODO: if only one alpha value is negative, should only that one be adjusted?
    alpha_l = alpha_r = dist(pts[0], pts[count-1]);
  }

  // Set all 4 control points and return the curve.
  CubicBezier2f fit;
  fit.pts[0] = pts[0];
  fit.pts[1] = pts[0] + endpoint_tangent_0 * alpha_l;
  fit.pts[2] = pts[count-1] + endpoint_tangent_1 * alpha_r;
  fit.pts[3] = pts[count-1];
  return fit;
}

}  // namespace pen
}  // namespace qi
