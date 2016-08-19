//
//  StrokeFitter.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation
import simd

class StrokeTouches {
  var first:StrokeTouch? = nil
  var touches = [[StrokeTouch]]()
  var predicted = [[StrokeTouch]]()
}

struct StrokeTouch {
  var position:float2
  
  init(position:float2) {
    self.position = position
  }
}

class StrokeFitter {
  let page: Page
  let stroke: Stroke
  var points = [float2]()
  var params = [Float]()
  var path = [StrokeSegment]()

  var touches = StrokeTouches()

  // Squared error in normalized screen coordinates.
  let errorThreshold = Float(0.00002)

  var predictedCount = Int(0)

  init(page: Page) {
    self.page = page
    self.stroke = page.newStroke()
  }

  func startStroke(touch:StrokeTouch) {
    touches.first = touch
    points.append(touch.position)
    params.append(0.0)
  }

  func continueStroke(touches:[StrokeTouch], predicted:[StrokeTouch] = []) {
    self.touches.touches.append(touches)
    self.touches.predicted.append(predicted)

    // Remove any points that were not actually sampled, only predicted.
    if predictedCount > 0 {
      points.removeLast(predictedCount)
      params.removeLast(predictedCount)
      predictedCount = 0
    }

    // Add new sampled points (only if they differ from the previous one).
    for touch in touches {
      let dist = distance(touch.position, points.last!)
      if dist > 0 {
        points.append(touch.position)
        params.append(params.last! + dist)
      }
    }

    // Add new predicted points (only if they differ from the previous one).
    if predicted.count > 0 {
      for touch in predicted {
        let dist = distance(touch.position, points.last!)
        if dist > 0 {
          predictedCount += 1
          points.append(touch.position)
          params.append(params.last! + dist)
        }
      }
    }

    // Sometimes the first several points are equal.  Don't call fitSampleRange() until there are
    // at least 2 distinct points (it would crash otherwise).
    if points.count < 2 { return }

    // Recursively compute a list of cubic Bezier segments.
    // TODO: don't recompute stable path segments near the beginning of the stroke.
    let leftTangent = points[1] - points[0];
    let rightTangent = points[points.count - 2] - points[points.count - 1];

    path.removeAll(keepCapacity: true)
    fitSampleRange(0..<points.count, leftTangent: leftTangent, rightTangent: rightTangent);

    page.setStrokePath(stroke, path: path)
  }

  func finishStroke() {
    assert(predictedCount == 0)
    page.finalizeStroke(stroke)
    print("Finished stroke with \(path.count) cubic beziers and length \(stroke.length)")
  }

  func applyStrokeTouches(touches:StrokeTouches) {
    assert(touches.first != nil)
    assert(touches.touches.count == touches.predicted.count)
    startStroke(touches.first!)
    for i in 0..<touches.touches.count {
      continueStroke(touches.touches[i], predicted:touches.predicted[i]);
      print("extended stroke to length: \(stroke.length)")
    }
    finishStroke()
    print("finished redoing stroke")
  }

  // TODO: investigate reparameterization.
  func fitSampleRange(range: Range<Int>, leftTangent: float2, rightTangent: float2) {
    assert(range.count > 1)

    let startIndex = range.startIndex
    let endIndex = range.endIndex

    if range.count == 2 {
      // Only two points... use a heuristic.
      // TODO: Double-check this heuristic (perhaps normalization needed?).
      // TODO: Perhaps this segment can be omitted entirely (perhaps blending
      //       endpoints of the adjacent segments.
      var line = Bezier3()
      line.pt0 = points[startIndex]
      line.pt3 = points[startIndex + 1]
      let oneThird = Float(1.0 / 3.0)
      line.pt1 = line.pt0 + (line.pt3 - line.pt0) * oneThird
      line.pt2 = line.pt3 + (line.pt0 - line.pt3) * oneThird
      path.append(StrokeSegment(line))
      return
    }

    // Normalize cumulative length between 0.0 and 1.0.
    let paramShift = -params[startIndex];
    let paramScale = 1.0 / (params[endIndex-1] + paramShift);

    let bez = fitBezier3ToPoints(points[startIndex..<endIndex],
                                 params: params[startIndex..<endIndex],
                                 paramShift: paramShift,
                                 paramScale: paramScale,
                                 startTangent: leftTangent,
                                 endTangent: rightTangent)

    // TODO: does Swift rounding/trunctation work like C?
    var splitIndex = (startIndex + endIndex) / 2
    var maxError = Float(0.0)
    for i in startIndex..<endIndex {
      let t = (params[i] + paramShift) * paramScale
      let diff = points[i] - bez.evaluate(t)
      let error = dot(diff, diff)
      if (error > maxError) {
        maxError = error;
        splitIndex = i;
      }
    }

    // The current fit is good enough... add it to the path and stop recursion.
    if (maxError < errorThreshold) {
      path.append(StrokeSegment(bez))
      return;
    }

    // Error is too large... split into two ranges and fit each.
    assert(splitIndex > startIndex && splitIndex < endIndex-1);
    var middleTangent1 = points[splitIndex] - points[splitIndex - 1]
    var middleTangent2 = points[splitIndex] - points[splitIndex + 1]
    if (dot(normalize(middleTangent1), normalize(middleTangent2)) < 0.5) {
      // The difference in directions is below threshold, so cause the two curve segments
      // to meet with the same slope.
      middleTangent1 = 0.5 * (points[splitIndex - 1] - points[splitIndex + 1])
      middleTangent2 = middleTangent1 * -1.0
    } else {
      // TODO: do something to correct this potentially unsightly direction change.
    }

    fitSampleRange(startIndex...splitIndex, leftTangent: leftTangent, rightTangent: middleTangent1)
    fitSampleRange(splitIndex..<endIndex, leftTangent: middleTangent2, rightTangent: rightTangent)
  }
}
