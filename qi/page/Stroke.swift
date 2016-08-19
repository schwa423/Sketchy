//
//  Stroke.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation
import simd

// TODO: document
class Stroke : CustomStringConvertible, Equatable {
  let index : Int
  var lengthAndReciprocal : float2
  var length : Float { get { return lengthAndReciprocal.x } }
  
  var path : [StrokeSegment] {
    get { return _path }
    set(newPath) {
      _path = newPath
      lengthAndReciprocal = float2(0.0, 0.0)
      for seg in _path {
        lengthAndReciprocal.x += seg.length
      }
      if (length > 0.0) {
        lengthAndReciprocal.y = 1.0 / length
      }
    }
  }
  private var _path = [StrokeSegment]()
  
  init(index: Int) {
    self.index = index
    lengthAndReciprocal = float2(0.0)
  }
  
  var description: String {
    var string = String("Stroke{\n  path: {")
    for seg in path {
      string += "\n    \(seg.description)"
    }
    string += path.isEmpty ? "}\n}" : "\n  }\n}"
    return string
  }
}

// Only identical Strokes are equal.
func == (lhs: Stroke, rhs: Stroke) -> Bool {
  return lhs === rhs
}