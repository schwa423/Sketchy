//
//  UITouchStrokeFitter.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation
import simd

extension float2 {
  init(_ pt: CGPoint) {
    self.init(x: Float(pt.x), y: Float(pt.y))
  }
}

extension UITouch {
  func normalizedLocationInView(view: UIView?) -> float2 {
    if let bounds = view?.bounds ?? window?.bounds {
      let width = CGRectGetWidth(bounds)
      let height = CGRectGetHeight(bounds)
      var pt = locationInView(view)
      
      // Scale so that the point is normalized to between [-1,1] in the smaller dimension, and
      // somewhat larger in the other dimension.
      // TODO: account for DPI, so that distance(pt1, pt2) == 1.0 means that the points are 1cm apart.
      if (width > height) {
        pt.x = (pt.x / width * 2.0 - 1.0) * width / height
        pt.y = (pt.y / height * -2.0 + 1.0)
      } else {
        pt.x = pt.x / width * 2.0 - 1.0
        pt.y = (pt.y / height * -2.0 + 1.0) * height / width
      }
      return float2(Float(pt.x), Float(pt.y))
    }
    assert(false)
    return float2(0.0, 0.0)
  }
  
  var location: CGPoint {
    get { return locationInView(self.view) }
  }
  
  var normalizedLocation: float2 {
    get { return normalizedLocationInView(self.view) }
  }
}

// Convenient wrapper for fitting strokes to UITouches.
class UITouchStrokeFitter : StrokeFitter {
  let view: UIView
  
  init(page:Page, view:UIView) {
    self.view = view
    super.init(page: page)
  }
  
  func startStroke(touch: UITouch) {
    startStroke(StrokeTouch(position: touch.normalizedLocation))
  }
  
  func continueStroke(touch: UITouch, withEvent event: UIEvent) {
    let actual = (event.coalescedTouchesForTouch(touch) ?? [touch]).map {
      StrokeTouch(position: $0.normalizedLocation)
    }
    let predicted = (event.predictedTouchesForTouch(touch) ?? []).map {
      StrokeTouch(position: $0.normalizedLocation)
    }
    continueStroke(actual, predicted:predicted)
  }
  
  func finishStroke(touch: UITouch, withEvent event: UIEvent) {
    continueStroke(touch, withEvent: event)
    finishStroke()
  }
}