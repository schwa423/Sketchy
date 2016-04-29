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

// Cubic Bezier curve.
struct Bezier3 : CustomStringConvertible {
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

  let firstPt = pts.first!
  let lastPt = pts.last!
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
