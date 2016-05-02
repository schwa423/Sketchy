//
//  StrokeFitter.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation
import simd

extension UITouch {
  func normalizedLocationInView(view: UIView?) -> CGPoint {
    if let bounds = view?.bounds ?? window?.bounds {
      var pt = locationInView(view)
      pt.x = pt.x / CGRectGetWidth(bounds) * 2.0 - 1.0
      pt.y = pt.y / CGRectGetHeight(bounds) * -2.0 + 1.0
      return pt
    }
    assert(false)
    return CGPoint(x: 0.0, y: 0.0)
  }
  
  var location: CGPoint {
    get { return locationInView(self.view) }
  }
  
  var normalizedLocation: CGPoint {
    get { return normalizedLocationInView(self.view) }
  }
}

extension float2 {
  init(_ pt: CGPoint) {
    self.init(x: Float(pt.x), y: Float(pt.y))
  }
}

class StrokeFitter {
  let page: Page
  let stroke: Stroke
  var points = [float2]()
  var params = [Float]()
  var path = [StrokeSegment]()
  let errorThreshold = Float(0.0002)
  
  var predictedCount = Int(0)
  
  init(page: Page) {
    self.page = page
    self.stroke = page.newStroke()
  }
  
  func startStroke(touch: UITouch) {
    points.append(float2(touch.normalizedLocation))
    params.append(0.0)
  }
  
  func continueStroke(touches: [UITouch], predicted: [UITouch] = []) {
    // Remove any points that were not actually sampled, only predicted.
    if predictedCount > 0 {
      points.removeLast(predictedCount)
      params.removeLast(predictedCount)
      predictedCount = 0
    }
    
    // Add new sampled points (only if they differ from the previous one).
    for touch in touches {
      let pt = float2(touch.normalizedLocation)
      let dist = distance(pt, points.last!)
      if dist > 0 {
        points.append(pt)
        params.append(params.last! + dist)
      }
    }
    
    // Add new predicted points (only if they differ from the previous one).
    if predicted.count > 0 {
      for touch in predicted {
        let pt = float2(touch.normalizedLocation)
        let dist = distance(pt, points.last!)
        if dist > 0 {
          predictedCount += 1
          points.append(pt)
          params.append(params.last! + dist)
        }
      }
    }
    
    // Sometimes the first several points are equal.  Don't call fitSampleRange() until there are
    // at least 2 distince points (it will crash otherwise).
    if points.count < 2 { return }
    
    // Recursively compute a list of cubic Bezier segments.
    // TODO: don't recompute stable path segments near the beginning of the stroke.
    let leftTangent = points[1] - points[0];
    let rightTangent = points[points.count - 2] - points[points.count - 1];
    
    path.removeAll(keepCapacity: true)
    fitSampleRange(0..<points.count, leftTangent: leftTangent, rightTangent: rightTangent);
    
    page.setStrokePath(stroke, path: path)
  }
  
  func finishStroke(touch: UITouch) {
    assert(predictedCount == 0)
    page.finalizeStroke(stroke)
    print("Finished stroke with \(path.count) cubic beziers and length \(stroke.length)")
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
    let middleTangent = points[splitIndex + 1] - points[splitIndex - 1];
    fitSampleRange(startIndex...splitIndex, leftTangent: leftTangent, rightTangent: middleTangent * -1.0);
    fitSampleRange(splitIndex..<endIndex, leftTangent: middleTangent, rightTangent: rightTangent);
  }
}