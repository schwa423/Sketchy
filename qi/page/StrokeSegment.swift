//
//  StrokeSegment.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation

// TODO: document
struct StrokeSegment : CustomStringConvertible, Equatable {
  let curve : Bezier3
  let reparam : Bezier3_1
  let length : Float
  
  init(_ curve : Bezier3) {
    self.curve = curve
    (reparam, length) = curve.arcLengthParameterization()
    
    // TODO: find better way to do this, or simply remove.
    validateFloat(length)
    validateFloat(reparam.pt0)
    validateFloat(reparam.pt1)
    validateFloat(reparam.pt2)
    validateFloat(reparam.pt3)
    validateFloat(curve.pt0.x)
    validateFloat(curve.pt1.x)
    validateFloat(curve.pt2.x)
    validateFloat(curve.pt3.x)
    validateFloat(curve.pt0.y)
    validateFloat(curve.pt1.y)
    validateFloat(curve.pt2.y)
    validateFloat(curve.pt3.y)
  }
  
  var description: String {
    return "length: \(length)  \(curve)"
  }
  
  private func validateFloat(f : Float) {
    assert(f == 0 || f/f == Float(1.0))
  }
}

func ==(lhs: StrokeSegment, rhs: StrokeSegment) ->Bool {
  if lhs.curve != rhs.curve { return false }
  if lhs.reparam != rhs.reparam { return false }
  if lhs.length != rhs.length { return false }
  return true
}