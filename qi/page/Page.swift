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
  var nextStrokeIndex = 0
  
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
    return Stroke(index: nextStrokeIndex++)
  }
  
  func setStrokePath(stroke: Stroke, path: [Bezier3]) {
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
  var path = [Bezier3]()

  // TODO: remove
  var fromFirebase: Bool = false
  
  init(index: Int) {
    self.index = index
  }
  
  var description: String {
    var string = String("Stroke{\n  path: {")
    for bez in path {
      string += "\n    \(bez.description)"
    }
    string += path.isEmpty ? "}\n}" : "\n  }\n}"
    return string
  }
}
