//
//  BezierFitter.mm
//  skeqi
//
//  Created by Josh Gargus on 5/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "BezierFitter.h"

#import <Metal/Metal.h>
#import <UIKit/UIKit.h>
#import "skeqi-Swift.h"

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

    // Compute distance between two points.
    float dist(Pt2f a, Pt2f b) {
      Pt2f diff = b - a;
      return sqrt(diff.dot(diff));
    }

    struct CubicBezier2f {
        Pt2f pts[4];

        Pt2f Evaluate(float t) {
            Pt2f tmp3[3];
            Pt2f tmp2[2];
            return Evaluate(t, tmp3, tmp2);
        }

        Pt2f Evaluate(float t, Pt2f* tmp3, Pt2f* tmp2) {
            float one_minus_t = 1.0 - t;
            tmp3[0] = pts[0] * one_minus_t + pts[1] * t;
            tmp3[1] = pts[1] * one_minus_t + pts[2] * t;
            tmp3[2] = pts[2] * one_minus_t + pts[3] * t;
            tmp2[0] = tmp3[0] * one_minus_t + tmp3[1] * t;
            tmp2[1] = tmp3[1] * one_minus_t + tmp3[2] * t;
            return tmp2[0] * one_minus_t + tmp2[1] * t;
        }

        std::pair<Pt2f, Pt2f> EvaluatePointAndNormal(float t) {
            Pt2f tmp3[3];
            Pt2f tmp2[2];
            Pt2f pt = Evaluate(t, tmp3, tmp2);
            Pt2f tangent = (tmp2[1] - tmp2[0]);
            tangent.normalize();
            // Rotate tangent clockwise by 90 degrees before returning.
            return std::make_pair(pt, Pt2f{-tangent[1], tangent[0]});
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
          Mat2f c = Mat2f::Zero();
          Pt2f x = Pt2f::Zero();

#define VECTORIZE 0
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
            // Alpha was negative, so use Wu/Barsky heuristic to place points.
            // TODO: if only one alpha value is negative, should only that one be adjusted?
            alpha_l = alpha_r = dist(pts.front(), pts.back());
          }

          // Set all 4 control points and return the curve.
          CubicBezier2f fit;
          fit.pts[0] = pts.front();
          fit.pts[1] = pts.front() + endpoint_tangent_0 * alpha_l;
          fit.pts[2] = pts.back() + endpoint_tangent_1 * alpha_r;
          fit.pts[3] = pts.back();
          return fit;
        }
    };  // class CubicBezier2f

    struct StrokeVertex {
      float px, py, pz, pw;
      float nx, ny, length;
      unsigned char cb, cg, cr, ca;
    };

}  // namespace qi

using namespace qi;

@implementation BezierFitter
{
  std::vector<Pt2f> pts_;
  id<PageProtocol> page_;
}

- (id)init {
    self = [super init];
    if (self) {
      page_ = Nil;
    }
    return self;
}

- (void)reset:(Page*) page {
  // TODO DCHECK(page_ == Nil);
  page_ = page;
}

- (void)add:(CGPoint)point {
  pts_.push_back({point.x, point.y});
}

- (void)finish {
  // TODO DCHECK(page_ != Nil);

  int num_pts = pts_.size();
  if (num_pts < 5) {
    std::cerr << "Not enough input points.";
    return;
  }

  // Compute chord-length parameterization for input points.
  std::vector<float> param;
  param.reserve(num_pts);
  Pt2f last_pt = pts_[0];
  float last_dist = 0.0;
  for (Pt2f pt : pts_) {
    last_dist = last_dist + dist(pt, last_pt);
    last_pt = pt;
    param.push_back(last_dist);
  }
  // Normalize cumulative length between 0.0 and 1.0.
  float param_scale = 1.0 / last_dist;
  float* param_floats = &(param[0]);
#pragma clang loop vectorize(enable) interleave(enable)
  for (int i = 0; i < num_pts; ++i) {
    param_floats[i] *= param_scale;
  }

  CubicBezier2f bez = CubicBezier2f::Fit(
      pts_, param, pts_[1] - pts_[0], pts_[pts_.size() - 2] - pts_[pts_.size() - 1]);
  bez.Print();

  // Tesselate 1000 vertices.
  // TODO compute appropriate #verts
  const static int kVertexCount = 1000;

  // TODO DCHECK(sizeof(StrokeVertex) == page->vertexSize)
  id<MTLDevice> device = page_.metalLibrary.device;
  size_t bufferLength = kVertexCount * page_.vertexSize;
  id<MTLBuffer> buffer = [device newBufferWithLength: bufferLength options: 0];
  auto verts = reinterpret_cast<StrokeVertex*>([buffer contents]);

  float incr = 1.0 / (kVertexCount - 2);
  for (int i = 0; i < kVertexCount; i += 2) {
    // We increment index by 2 each loop iteration, so the last iteration will have
    // "index == kVertexCount - 2", and therefore a parameter value of "i * incr == 1.0".
    std::pair<Pt2f, Pt2f> pt = bez.EvaluatePointAndNormal(i * incr);
      float px = pt.first[0];
      float py = pt.first[1];
      float nx = pt.second[0];
      float ny = pt.second[1];
    verts[i].px = verts[i+1].px = pt.first[0];
    verts[i].py = verts[i+1].py = pt.first[1];
    verts[i].pz = verts[i+1].pz = 0.0;
    verts[i].pw = verts[i+1].pw = 1.0;
    verts[i].nx = -pt.second[0];
    verts[i].ny = -pt.second[1];
    verts[i+1].nx = pt.second[0];
    verts[i+1].ny = pt.second[1];

    // TODO this is a hack since our vertex shader doesn't actually use the specified normals.
    float kWidth = 0.02;
    verts[i].px += nx * kWidth;
    verts[i].py += ny * kWidth;
    verts[i+1].px -= nx * kWidth;
    verts[i+1].py -= ny * kWidth;

    // TODO length
    verts[i].length = verts[i+1].length = 0.0;
    verts[i].cb = verts[i+1].cb = 0;
    verts[i].cg = verts[i+1].cg = 0;
    verts[i].cr = verts[i+1].cr = 1;
    verts[i].ca = verts[i+1].ca = 1;
  }

  auto stroke = [[Stroke2 alloc] init];
  stroke.buffer = buffer;
  stroke.vertexCount = kVertexCount;

  [page_ addStroke: stroke];
  pts_.clear();
}

+ (void)benchmark {
  BezierFitter* fitter = [[BezierFitter alloc] init];
  if (!fitter) {
    std::cerr << "BezierFitter::benchmark() Could not instantiate fitter";
    return;
  }

  CubicBezier2f bez;
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

  std::vector<Pt2f> pts;
  std::vector<float> parameterization;
  for (float i = 0.0; i <= 1.0; i += 0.0005) {
    pts.push_back(bez.Evaluate(i));
    parameterization.push_back(i);
  }
  std::cerr << "Created uniform parameterization for " << pts.size() << " sample-points";

  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();

  CubicBezier2f fit;
  static const int kBenchmarkIterations = 1000;
  for (int i = 0; i < kBenchmarkIterations; ++i) {
    fit = CubicBezier2f::Fit(
        pts, parameterization, bez.pts[1] - bez.pts[0], bez.pts[3] - bez.pts[2]);
  }

  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cerr << "Fit Bezier " << kBenchmarkIterations << " times in "
            << elapsed_seconds.count() << " seconds.";
  fit.Print();
}

@end
