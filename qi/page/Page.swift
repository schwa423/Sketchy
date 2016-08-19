//
//  Page.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation
import simd

// TODO: document
class Page {
  var strokes = [Stroke]()
  var dirtyStrokes = [Stroke]()
  
  func nextStrokeIndex() -> Int {
    _nextStrokeIndex += 1
    return _nextStrokeIndex - 1
  }
  private var _nextStrokeIndex : Int = 0
  
  // TODO: use weak reference to observers?
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
  
  func clear() {
    strokes.removeAll()
    dirtyStrokes.removeAll()
    _nextStrokeIndex = 0
  }
}
