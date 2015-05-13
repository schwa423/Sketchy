//
//  BezierFitter.mm
//  skeqi
//
//  Created by Josh Gargus on 5/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "BezierFitter.h"

#include <chrono>
#include <iostream>
#include <vector>

#include <Eigen/Core>


namespace qi {

    typedef Eigen::Vector2d Pt2d;  // typedef Matrix< double, 2, 1 >  (column vector)
    typedef Eigen::Vector2f Pt2f;
    typedef Eigen::Matrix2f Mat2f;

    typedef Eigen::Vector4f Vec4f;
    typedef Eigen::Array<float, 1, 4> Arr4f;
    typedef Eigen::Array<float, 1, 8> Arr8f;

    struct CubicBezier2f {
        Pt2f pts[4];

        Pt2f Evaluate(float t) {
            Pt2f temp[3];
            float one_minus_t = 1.0 - t;
            temp[0] = pts[0] * one_minus_t + pts[1] * t;
            temp[1] = pts[1] * one_minus_t + pts[2] * t;
            temp[2] = pts[2] * one_minus_t + pts[3] * t;
            temp[0] = temp[0] * one_minus_t + temp[1] * t;
            temp[1] = temp[1] * one_minus_t + temp[2] * t;
            return temp[0] * one_minus_t + temp[1] * t;
        }

        void Print() {
          std::cerr << "p0(" << pts[0][0] << "," << pts[0][1] << "), "
                    << "p1(" << pts[1][0] << "," << pts[1][1] << "), "
                    << "p2(" << pts[2][0] << "," << pts[2][1] << "), "
                    << "p3(" << pts[3][0] << "," << pts[3][1] << ")" << std::endl;
        }

        static CubicBezier2f Fit(std::vector<Pt2f>& pts,
                                 std::vector<float>& parameterization,
                                 Pt2f endpoint_tangent_0,
                                 Pt2f endpoint_tangent_1) {
          CubicBezier2f fit;
          fit.pts[0] = Pt2f::Zero();
          fit.pts[1] = Pt2f::Zero();
          fit.pts[2] = Pt2f::Zero();
          fit.pts[3] << 0.11, 0.22;

          Mat2f c = Mat2f::Zero();
          Pt2f x = Pt2f::Zero();

#define VECTORIZE 1
#if !VECTORIZE
          for (int i = 0; i < pts.size(); ++i) {
            float t = parameterization[i];
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
            Pt2f tmp = pts[i] - (pts.front() * (b0 + b1) + pts.back() * (b2 + b3));
            x[0] += a0.dot(tmp);
            x[1] += a1.dot(tmp);
          }
#else  // VECTORIZE

          // Requires -ffast-math.  TODO: experiment with fp_contract pragma (maybe globally
          // enabling -ffast-math isn't necessary).

          int end_index = static_cast<int>(pts.size()) - 1;
          float* params = &(parameterization[0]);
          float* raw_pts = reinterpret_cast<float*>(&(pts[0]));
          float first_valx = pts.front()[0];
          float first_valy = pts.front()[1];
          float last_valx = pts.back()[0];
          float last_valy = pts.back()[1];

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
          for (int i = 0; i <= end_index; i++) {
            float t = params[i];
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
            float tmpx = raw_pts[i*2] - first_valx * b0b1 - last_valx * b2b3;
            float tmpy = raw_pts[i*2+1] - first_valy * b0b1 - last_valy * b2b3;
            x0[i] = a0x * tmpx + a0y * tmpy;  // a0.dot(tmp);
            x1[i] = a1x * tmpx + a1y * tmpy;  // a1.dot(tmp);
          }

          // Accumulate sums.  If we try to do this in the loop above, the vectorizer fails.
#pragma clang loop vectorize(enable) interleave(enable)
          for (int i = 0; i <= end_index; i++) {
            c(0,0) += c00[i];
            c(0,1) += c01[i];
            c(1,1) += c11[i];
            x[0] += x0[i];
            x[1] += x1[i];
          }
#endif  // VECTORIZE


          c(1,0) = c(0,1);

          float det_c0_c1 = c(0,0) * c(1,1) - c(1,0) * c(0,1);
          float det_c0_x = c(0,0)   * x[1] - c(0,1)   * x[0];
          float det_x_c1 = x[0]     * c(1,1) - x[1]   * c(0,1);

          if (det_c0_c1 == 0.0)
            det_c0_c1 = c(0,0) * c(1,1) * 10e-12;


          // Compute alpha values used to determine the distance along the left/right tangent
          // vectors to place the middle two control points.  If either alpha value is negative,
          // recompute it using Wu/Barsky heuristic.
          float alpha_l = det_x_c1 / det_c0_c1;
          float alpha_r = det_c0_x / det_c0_c1;
          if (alpha_l < 0.0 || alpha_r < 0.0) {
            // Alpha was nagative, so use Wu/Barsky heuristic to place points.
            Pt2f diff = pts.front() - pts.back();
            float dist = sqrt(diff.dot(diff));
            // TODO: if only one alpha value is negative, should only that one be adjusted?
            alpha_l = alpha_r = dist;
          }

          // Set all 4 control points and return the curve.
          fit.pts[0] = pts.front();
          fit.pts[1] = pts.front() + endpoint_tangent_0 * alpha_l;
          fit.pts[2] = pts.back() + endpoint_tangent_1 * alpha_r;
          fit.pts[3] = pts.back();
          return fit;
        }
    };

}  // namespace qi


@implementation BezierFitter

- (id)init {
    self = [super init];
    if (self) {
        qi::CubicBezier2f bez;
        bez.pts[0][0] = -1.0;
        bez.pts[0][1] = -1.0;
        bez.pts[1][0] = -0.5;
        bez.pts[1][1] = 0.0;
        bez.pts[2][0] = 0.5;
        bez.pts[2][1] = 0.0;
        bez.pts[3][0] = 1.0;
        bez.pts[3][1] = 1.0;

        std::cerr << "Initial Bezier: ";
        bez.Print();

        std::vector<qi::Pt2f> pts;
        std::vector<float> parameterization;
        for (float i = 0.0; i <= 1.0; i += 0.0005) {
          pts.push_back(bez.Evaluate(i));
          parameterization.push_back(i);

        }

        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        start = std::chrono::high_resolution_clock::now();

        qi::CubicBezier2f fit;
        for (int i = 0; i < 1000; ++i) {
          fit = qi::CubicBezier2f::Fit(
              pts, parameterization, bez.pts[1] - bez.pts[0], bez.pts[3] - bez.pts[2]);
        }

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        std::cerr << "Fit Bezier in " << elapsed_seconds.count() << " seconds:  ";
        fit.Print();
    }
    return self;
}

@end
