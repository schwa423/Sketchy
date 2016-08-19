//
//  RenderableStroke.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation

// TODO: document
class RenderableStroke : Stroke {
  var vertexCount: Int = 0
  var offset: Int = 0
  var buffer: MTLBuffer?
  
  override init(index: Int) {
    super.init(index: index)
  }
  
  func draw(encoder: MTLRenderCommandEncoder, time: Float) {
    if (vertexCount > 0) {
      encoder.setVertexBuffer(buffer, offset: offset, atIndex: 0)
      
      var strokeParams = StrokeParams(length: length, width: 0.05, time: time)
      encoder.setVertexBytes(&strokeParams, length: sizeof(StrokeParams), atIndex: 2);
      
      // Time not set here, it is the same for all strokes so it is set in RenderablePage.draw().
      
      encoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount)
    }
  }
  
  override var description: String {
    var string = String("RenderableStroke{\n  vertexCount: \(vertexCount)\n  path: {")
    for bez in path {
      string += "\n    \(bez.description)"
    }
    string += path.isEmpty ? "}\n}" : "\n  }\n}"
    return string
  }
}

// Corresponds to the StrokeParams struct consumed by the vertex shaders.
// See RenderablePage.
private struct StrokeParams {
  // Domain of amplitude is [0, 0.5] because range of sin+1 is [0,2].
  let length, reciprocalLength, width, reciprocalWidth, time : Float
  
  init(length: Float, width: Float, time: Float) {
    self.length = length
    self.reciprocalLength = 1.0 / length
    self.width = width
    self.reciprocalWidth = 1.0 / width
    self.time = time
  }
}
