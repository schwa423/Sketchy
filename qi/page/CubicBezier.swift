import simd

struct LineSegment {
  var pt0 = float2()
  var pt1 = float2()

  @warn_unused_result
  func evaluate(t: Float) -> float2 {
    return pt0 * (1.0 as Float - t) + pt1 * t;
  }
}

// Quadratic Bezier curve.
struct Bezier2 {
  var pt0 = float2()
  var pt1 = float2()
  var pt2 = float2()

  @warn_unused_result
  func evaluate(t: Float) -> float2 {
    let omt = Float(1.0) - t
    let tmp01 = pt0 * omt + pt1 * t
    let tmp12 = pt1 * omt + pt2 * t
    return tmp01 * omt + tmp12 * t;
  }
}

// 1D Cubic Bezier curve.
struct Bezier3_1 : Equatable {
  var pt0 = Float(0)
  var pt1 = Float(0)
  var pt2 = Float(0)
  var pt3 = Float(0)
  
  func evaluate(t: Float) -> Float {
    let omt = Float(1.0) - t
    let tmp0 = pt0 * omt + pt1 * t
    let tmp1 = pt1 * omt + pt2 * t
    let tmp2 = pt2 * omt + pt3 * t
    let tmp01 = tmp0 * omt + tmp1 * t
    let tmp12 = tmp1 * omt + tmp2 * t
    return tmp01 * omt + tmp12 * t
  }
}

// 2D Cubic Bezier curve.
struct Bezier3 : CustomStringConvertible, Equatable {
  var pt0 = float2()
  var pt1 = float2()
  var pt2 = float2()
  var pt3 = float2()

  @warn_unused_result
  func evaluate(t: Float) -> float2 {
    var bez2 = Bezier2()
    var line = LineSegment()
    return evaluate(t, line: &line, bez2: &bez2)
  }

  @warn_unused_result
  func evaluate(t: Float, inout line: LineSegment, inout bez2: Bezier2) -> float2 {
    let omt = Float(1.0) - t
    bez2.pt0 = pt0 * omt + pt1 * t
    bez2.pt1 = pt1 * omt + pt2 * t
    bez2.pt2 = pt2 * omt + pt3 * t
    line.pt0 = bez2.pt0 * omt + bez2.pt1 * t
    line.pt1 = bez2.pt1 * omt + bez2.pt2 * t
    return line.pt0 * omt + line.pt1 * t;
  }

  @warn_unused_result
  func split(t: Float) -> (first: Bezier3, second: Bezier3) {
    var bez2 = Bezier2()
    var line = LineSegment()
    let splitPoint = evaluate(t, line: &line, bez2: &bez2)

    return (Bezier3(pt0: pt0,
                    pt1: bez2.pt0,
                    pt2: line.pt0,
                    pt3: splitPoint),
            Bezier3(pt0: splitPoint,
                    pt1: line.pt1,
                    pt2: bez2.pt2,
                    pt3: pt3))
  }
  
  // Compute the cumulative arc length of the curve at the specified parameter value, using the
  // approach described in "Adaptive subdivision and the length of Bezier curves" by Jens Gravsen.
  // The insight is that the length is bounded below by the length of the line segment (pt0, pt3), 
  // and bounded above by the sum of the line segments (pt0, pt1), (pt1, pt2), (pt2, pt3).
  func arcLength(maxErrorRate maxErrorRate: Float = 0.01, debugDepth: Int = 0) -> Float {
    assert(maxErrorRate > 0.0)
    assert(debugDepth < 100)  // TODO: some input triggers an infinite recursion.

    let upperBound = distance(pt0, pt1) + distance(pt1, pt2) + distance(pt2, pt3);
    let lowerBound = distance(pt0, pt3);
    
    if (!(upperBound > 0.0)) {
      // Catch zero and NaN.
      return 0.0
    } else if ((upperBound - lowerBound) / upperBound <= maxErrorRate * 2.0) {
      // Returning the mean of the two bounds guarantees the result is within the error tolerance.
      return 0.5 * (upperBound + lowerBound);
    } else if (upperBound < 0.000005) {
      // In some cases, floating point precision will result in non-convergence;
      // when this is detected, we explicitly terminate recursion.
      // TODO: Is a constant threshold the right approach?  Is this the right threshold?
      return 0.5 * (upperBound + lowerBound);
    } else {
      // This curve is not flat enough.  Split into two curves, and recursively evaluate the length
      // of each one.
      let pair = split(0.5)
      let len0 = pair.0.arcLength(maxErrorRate: maxErrorRate, debugDepth: debugDepth + 1)
      let len1 = pair.1.arcLength(maxErrorRate: maxErrorRate, debugDepth: debugDepth + 1)
      return len0 + len1;
    }
  }
  
  // Compute an arc-length reparameterization of this curve.  In other words, the following code:
  //   let bez3 : Bezier3 = obtainBezier3()
  //   let reparam = bez3.arcLengthParameterization()
  //   var points = [float2]()
  //   for t in 0.stride(through: 1.0, by: 0.1) {
  //     points.append(bez.evaluate(reparam.evaluate(t)))
  //   }
  // ... results in a collection of points that are approximately equally-spaced along bez3.
  //
  // Uses a simplified version of "Approximate Arc Length Parameterization" by Walter and Fournier.
  // In particular, this version does not detect cases where it would be desirable to split the
  // reparameterization curve into two segments.
  func arcLengthParameterization() -> (Bezier3_1, Float) {
    let fullLength = arcLength()
    let oneThirdLength = split(1.0 / 3.0).0.arcLength()
    let twoThirdsLength = split(2.0 / 3.0).0.arcLength()
    let normalizer : Float = 1.0 / fullLength
    let s0 = oneThirdLength * normalizer
    let s1 = twoThirdsLength * normalizer
    var bez = Bezier3_1()
    bez.pt0 = 0.0
    bez.pt1 = (18.0*s0 - 9.0*s1 + 2.0) / 6.0
    bez.pt2 = (-9.0*s0 + 18.0*s1 - 5.0) / 6.0
    bez.pt3 = 1.0
    return (bez, fullLength)
  }

  var description: String {
    get { return "Bezier3[(\(pt0.x),\(pt0.y)) (\(pt1.x),\(pt1.y)) (\(pt2.x),\(pt2.y)) (\(pt3.x),\(pt3.y))]"; }
  }
}

@warn_unused_result
func fitBezier3ToPoints(pts: ArraySlice<float2>,
                        params: ArraySlice<Float>,
                        paramShift: Float,
                        paramScale: Float,
                        startTangent: float2,
                        endTangent: float2) -> Bezier3 {
  var c00, c01, c10, c11, x0, x1 : Float
  c00 = 0; c01 = 0; c10 = 0; c11 = 0; x0 = 0; x1 = 0;

  assert(pts.startIndex == params.startIndex)
  assert(pts.endIndex == params.endIndex)
  assert(!startTangent.x.isNaN && !startTangent.y.isNaN)
  assert(!endTangent.x.isNaN && !endTangent.y.isNaN)
  let firstPt = pts[pts.startIndex]
  let lastPt = pts[pts.endIndex - 1]
  assert(!firstPt.x.isNaN && !firstPt.y.isNaN)
  assert(!lastPt.x.isNaN && !lastPt.y.isNaN)
  
  for i in pts.startIndex..<pts.endIndex {
    let t = (params[i] + paramShift) * paramScale
    let omt = Float(1.0) - t
    let b0 = omt * omt * omt
    let b1 = 3.0 * t * omt * omt
    let b2 = 3.0 * t * t * omt
    let b3 = t * t * t
    let a0 = startTangent * b1
    let a1 = endTangent * b2
    c00 += dot(a0, a0)
    c01 += dot(a0, a1)
    // c10 == dot(a1, a0) == c01, so don't compute here,
    // but instead set it just after the loop.
    c11 += dot(a1, a1)
    let tmp = pts[i] - (firstPt * (b0 + b1) + lastPt * (b2 + b3))
    x0 += dot(a0, tmp)
    x1 += dot(a1, tmp)
  }
  // Symmetric.  See comment in loop.
  c10 = c01

  var det_c0_c1 = c00  * c11  -  c10  * c01
  let det_c0_x =  c00  * x1 -  c01  * x0
  let det_x_c1 =  x0 * c11  -  x1 * c01

  if (det_c0_c1 == 0.0) {
    det_c0_c1 = c00 * c11 * 10e-12
  }

  // Compute alpha values used to determine the distance along the left/right tangent
  // vectors to place the middle two control points.  If either alpha value is negative,
  // recompute it using Wu/Barsky heuristic.
  var alpha_l = det_x_c1 / det_c0_c1
  var alpha_r = det_c0_x / det_c0_c1
  assert(!alpha_l.isNaN && !alpha_r.isNaN)
  if (alpha_l < 0.0 || alpha_r < 0.0) {
    // Alpha was negative, so use Wu/Barsky heuristic to place points.
    // TODO: if only one alpha value is negative, should only that one be adjusted?
    alpha_l = distance(firstPt, lastPt)
    alpha_r = alpha_l
  }
  
  // Set all 4 control points and return the curve.
  return Bezier3(pt0: firstPt,
                 pt1: firstPt + startTangent * alpha_l,
                 pt2: lastPt + endTangent * alpha_r,
                 pt3: lastPt)
}

// TODO: put this somewhere suitable
func ==(lhs: float2, rhs: float2) -> Bool {
  return lhs.x == rhs.x && lhs.y == rhs.y
}

func ==(lhs: Bezier3, rhs: Bezier3) -> Bool {
  return lhs.pt0 == rhs.pt0 && lhs.pt1 == rhs.pt1 && lhs.pt2 == rhs.pt2 && lhs.pt3 == rhs.pt3
}

func ==(lhs: Bezier3_1, rhs: Bezier3_1) -> Bool {
  return lhs.pt0 == rhs.pt0 && lhs.pt1 == rhs.pt1 && lhs.pt2 == rhs.pt2 && lhs.pt3 == rhs.pt3
}
