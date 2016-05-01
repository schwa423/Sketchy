//
//  Page.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation

// TODO: document
class PageObserver : Equatable {
  func onFinalizeStroke(stroke: Stroke!) -> Void {}
}

// PageObservers are equal only if they are identical.
func ==(lhs: PageObserver, rhs: PageObserver) -> Bool {
  return lhs === rhs
}

// TODO: document
class Page {
  var strokes = [Stroke]()
  var dirtyStrokes = [Stroke]()
  
  func nextStrokeIndex() -> Int {
    _nextStrokeIndex += 1
    return _nextStrokeIndex - 1
  }
  private var _nextStrokeIndex : Int = 0
  
  private var observers = [PageObserver]()
  
  // Add an observer that is called whenever a Stroke is finalized.
  func addObserver(observer: PageObserver!) { observers.append(observer) }
  func removeObserver(observer: PageObserver!) {
    if let index = observers.indexOf(observer) {
      observers.removeAtIndex(index)
    }
  }
  
  func newStroke() -> Stroke {
    let stroke = instantiateStroke()
    strokes.append(stroke)
    return stroke
  }
  
  func instantiateStroke() -> Stroke {
    return Stroke(index: nextStrokeIndex())
  }
  
  func setStrokePath(stroke: Stroke, path: [StrokeSegment]) {
    assert(strokes[stroke.index] === stroke);
    stroke.path = path
    dirtyStrokes.append(stroke)
  }
  
  func finalizeStroke(stroke: Stroke) {
    // TODO: perhaps put all strokes into one shared buffer.
    
    for observer in observers {
      observer.onFinalizeStroke(stroke)
    }
  }
}

// TODO: document
class Stroke : CustomStringConvertible {
  let index: Int
  var length = Float(0)
  
  var path : [StrokeSegment] {
    get { return _path }
    set(newPath) {
      _path = newPath
      length = 0
      for seg in _path {
        length += seg.length
      }
    }
  }
  var _path = [StrokeSegment]()

  // TODO: remove
  var fromFirebase: Bool = false
  
  init(index: Int) {
    self.index = index
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

func validateFloat(f : Float) {
  assert(f == 0 || f/f == Float(1.0))
}

// TODO: document
struct StrokeSegment : CustomStringConvertible {
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
}
